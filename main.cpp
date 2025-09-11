#include <QCoreApplication>
#include <QTimer>
#include <QElapsedTimer>
#include <QDebug>
#include <QDateTime>
#include <QProcess>
#include "synchapi.h"
#include <QSettings>
#include <windows.h>
#include <iostream>
#include "logger.h"

int iIntervalInSeconds;
int iMinimumIntervalInSeconds;
int iIntervalBetweenCommands;
int iCounterThreshold;
int iCounter = 0;
int iMemoryLimit;
int iLowMemoryMemoryLimit;
int iLowMemoryCheckIntervalInSeconds;
double dFreeMem;
//QTimer timer;
QTimer checkLowMemoryTimer;
QElapsedTimer elapsedTimer;
Logger logger("c:\\RAMMapRunner.log");
const double dBytesToMB = 1024.0 * 1024.0;

void readSettings()
{
    qApp->setOrganizationName("andreag");
    qApp->setApplicationName("RAMMapRunner");
    QSettings settings;
    bool bSettingExists;
    bSettingExists = settings.contains ("iIntervalInSeconds");
    if (!bSettingExists) settings.setValue ("iIntervalInSeconds", 30);
    iIntervalInSeconds = settings.value ("iIntervalInSeconds", 10).toInt ();
    bSettingExists = settings.contains ("iMinimumIntervalInSeconds");
    if (!bSettingExists) settings.setValue ("iMinimumIntervalInSeconds", 30);
    iMinimumIntervalInSeconds = settings.value ("iMinimumIntervalInSeconds", 10).toInt ();
    bSettingExists = settings.contains ("iIntervalBetweenCommands");
    if (!bSettingExists) settings.setValue ("iIntervalBetweenCommands", 1000);
    iIntervalBetweenCommands = settings.value ("iIntervalBetweenCommands", 1000).toInt ();
    bSettingExists = settings.contains ("iCounterThreshold");
    if (!bSettingExists) settings.setValue ("iCounterThreshold", 10);
    iCounterThreshold = settings.value ("iCounterThreshold", 10).toInt ();
    bSettingExists = settings.contains ("iMemoryLimit");
    if (!bSettingExists) settings.setValue ("iMemoryLimit", 1280);
    iMemoryLimit = settings.value ("iMemoryLimit", 1280).toInt ();
    bSettingExists = settings.contains ("iLowMemoryMemoryLimit");
    if (!bSettingExists) settings.setValue ("iLowMemoryMemoryLimit", 1024);
    iLowMemoryMemoryLimit = settings.value ("iLowMemoryMemoryLimit", 1024).toInt ();
    bSettingExists = settings.contains ("iLowMemoryCheckIntervalInSeconds");
    if (!bSettingExists) settings.setValue ("iLowMemoryCheckIntervalInSeconds", 10);
    iLowMemoryCheckIntervalInSeconds = settings.value ("iLowMemoryCheckIntervalInSeconds", 10).toInt ();
    // logger.write("iIntervalInSeconds " +QString::number (iIntervalInSeconds));
    // logger.write("iIntervalBetweenCommands " +QString::number (iIntervalBetweenCommands));
    // logger.write("iCounterThreshold " +QString::number (iCounterThreshold));
    // logger.write("iMemoryLimit " +QString::number (iMemoryLimit));
}

// Worker class separated from main
class Worker : public QObject
{
    Q_OBJECT

public slots:

    int runCommands()
    {
        QStringList commands = { "-Ew", "-Es", "-Em", "-Et", "-E0" };
        QString sBaseCommand = "C:\\RAMOptimizer\\RAMMap.exe";
        int retCode = 0;
        for (int i = 0; i < commands.size(); ++i)
        {
            retCode = QProcess::execute(sBaseCommand + " " + commands[i]);
            if (i < commands.size() - 1)
                Sleep(iIntervalBetweenCommands);
        }
        dFreeMem = getFreeRAM();
        logger.write("Free RAM after cleanup: " + QString::number(dFreeMem));
        return retCode;
        // int retCode;
        // QString sCommand;
        //        // -Ewsmt0
        // sCommand = "C:\\RAMOptimizer\\RAMMap.exe -Ew";
        // retCode = QProcess::execute(sCommand);
        // Sleep(iIntervalBetweenCommands);
        ////        qDebug() << "Processo terminato con codice:" << retCode;
        // sCommand = "C:\\RAMOptimizer\\RAMMap.exe -Es";
        // retCode = QProcess::execute(sCommand);
        // Sleep(iIntervalBetweenCommands);
        ////        qDebug() << "Processo terminato con codice:" << retCode;
        // sCommand = "C:\\RAMOptimizer\\RAMMap.exe -Em";
        // retCode = QProcess::execute(sCommand);
        // Sleep(iIntervalBetweenCommands);
        ////        qDebug() << "Processo terminato con codice:" << retCode;
        // sCommand = "C:\\RAMOptimizer\\RAMMap.exe -Et";
        // retCode = QProcess::execute(sCommand);
        // Sleep(iIntervalBetweenCommands);
        ////        qDebug() << "Processo terminato con codice:" << retCode;
        // sCommand = "C:\\RAMOptimizer\\RAMMap.exe -E0";
        // retCode = QProcess::execute(sCommand);
        ////        Sleep(iIntervalBetweenCommands);
        // dFreeMem = getFreeRAM ();
        // logger.write("Free RAM after cleanup: " + QString::number (dFreeMem));
        // return retCode;
    }

    void doWork()
    {
        // qint64 elapsedTimeMs = elapsedTimer.elapsed();
        // double elapsedTimeSec = static_cast<double>(elapsedTimeMs) / 1000.0;
        // if (elapsedTimeSec < iMinimumIntervalInSeconds) return;
        readSettings();
        //timer.setInterval (iIntervalInSeconds * 1000);
        checkLowMemoryTimer.setInterval (iLowMemoryCheckIntervalInSeconds * 1000);
        dFreeMem = getFreeRAM ();
        qDebug() << "Free RAM: " << dFreeMem;
        // logger.write("Free RAM: " + QString::number (dFreeMem));
        iCounter++;
        if (dFreeMem < iMemoryLimit && iCounter >= iCounterThreshold)
        {
            qDebug() << "iCounter: " << iCounter;
            doMemoryWork ();
            // return;
        }
        else if (dFreeMem < iLowMemoryMemoryLimit)
        {
            qDebug() << "iCounter: " << iCounter;
            doLowMemoryWork ();
            // return;
        }
        //elapsedTimer.start ();
    }

    void doMemoryWork()
    {
        // qint64 elapsedTimeMs = elapsedTimer.elapsed();
        // double elapsedTimeSec = static_cast<double>(elapsedTimeMs) / 1000.0;
        // if (elapsedTimeSec < iMinimumIntervalInSeconds) return;
        iCounter = 0;
        qDebug() << "doMemoryWork Job started at:" << QDateTime::currentDateTime().toString();
        logger.write("Free RAM: " + QString::number (dFreeMem));
        logger.write("doMemoryWork Job started");
        int retCode = runCommands();
        qDebug() << "Processo terminato con codice:" << retCode;
    }

    void doLowMemoryWork()
    {
        qint64 elapsedTimeMs = elapsedTimer.elapsed();
        double elapsedTimeSec = static_cast<double>(elapsedTimeMs) / 1000.0;
        if (elapsedTimeSec < iMinimumIntervalInSeconds) return;
        // readSettings();
        //elapsedTimer.stop ();
        iCounter = 0;
        qDebug() << "doLowMemoryWork Job started at:" << QDateTime::currentDateTime().toString();
        logger.write("Free RAM: " + QString::number (dFreeMem));
        logger.write("doLowMemoryWork Job started");
        int retCode = runCommands();
        qDebug() << "Processo terminato con codice:" << retCode;
        elapsedTimer.start ();
    }

    double getFreeRAM()
    {
        // Declare a MEMORYSTATUSEX structure
        MEMORYSTATUSEX status;
        // Set the dwLength member to the size of the structure
        status.dwLength = sizeof(status);
        // Call the function to populate the structure
        if (GlobalMemoryStatusEx(&status))
        {
            // The function succeeded, now you can access the data
            // ullAvailPhys is the free physical RAM in bytes
            long long free_ram_bytes = status.ullAvailPhys;
            //std::cout << "Available Physical RAM: " << free_ram_bytes / dBytesToMB << " MB" << std::endl;
            double dFreeRam = free_ram_bytes / dBytesToMB;
            return dFreeRam;
        }
        else
        {
            // The function failed, you can get the error code
            std::cerr << "Failed to retrieve memory status. Error code: " << GetLastError() << std::endl;
            return 0.0;
        }
    }
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setOrganizationName("andreag");
    app.setApplicationName("RAMMapRunner");
    logger.write("Application started");
    readSettings();
    Worker worker;
    // QTimer timer;
    // Connect the timer timeout to the worker's job
    // QObject::connect(&timer, SIGNAL(timeout()), &worker, SLOT(doWork()));
    // timer.start(iIntervalInSeconds * 1000);
    QObject::connect(&checkLowMemoryTimer, SIGNAL(timeout()), &worker, SLOT(doWork()));
    checkLowMemoryTimer.start(iLowMemoryCheckIntervalInSeconds * 1000);
    elapsedTimer.start ();
    return app.exec();
}

#include "main.moc"
