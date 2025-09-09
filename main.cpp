#include <QCoreApplication>
#include <QTimer>
#include <QDebug>
#include <QDateTime>
#include <QProcess>
#include "synchapi.h"
#include <QSettings>
#include <windows.h>
#include <iostream>

int iIntervalInSeconds;
int iIntervalBetweenCommands;
int iCounterThreshold;
int iCounter = 0;
int iMemoryLimit;
QTimer timer;

void readSettings()
{
    qApp->setOrganizationName("andreag");
    qApp->setApplicationName("RAMMapRunner");
    QSettings settings;
    bool bSettingExists;
    bSettingExists = settings.contains ("iIntervalInSeconds");
    if (!bSettingExists) settings.setValue ("iIntervalInSeconds", 30);
    iIntervalInSeconds = settings.value ("iIntervalInSeconds", 10).toInt ();
    bSettingExists = settings.contains ("iIntervalBetweenCommands");
    if (!bSettingExists) settings.setValue ("iIntervalBetweenCommands", 1000);
    iIntervalBetweenCommands = settings.value ("iIntervalBetweenCommands", 1000).toInt ();
    bSettingExists = settings.contains ("iCounterThreshold");
    if (!bSettingExists) settings.setValue ("iCounterThreshold", 10);
    iCounterThreshold = settings.value ("iCounterThreshold", 10).toInt ();
    bSettingExists = settings.contains ("iMemoryLimit");
    if (!bSettingExists) settings.setValue ("iMemoryLimit", 1024);
    iMemoryLimit = settings.value ("iMemoryLimit", 1024).toInt ();
}

// Worker class separated from main
class Worker : public QObject
{
    Q_OBJECT

public slots:

    void doWork()
    {
        readSettings();
        timer.setInterval (iIntervalInSeconds * 1000);
        double dFreeMem = getFreeRAM ();
        qDebug() << "Free RAM: " << dFreeMem;
        if (dFreeMem > iMemoryLimit && iCounter < iCounterThreshold)
        {
            iCounter++;
            qDebug() << "iCounter: " << iCounter;
            return;
        }
        iCounter = 0;
        qDebug() << "Job stated at:" << QDateTime::currentDateTime().toString();
        int retCode;
        QString sCommand;
        // -Ewsmt0
        sCommand = "C:\\RAMOptimizer\\RAMMap.exe -Ew";
        retCode = QProcess::execute(sCommand);
        Sleep(iIntervalBetweenCommands);
        qDebug() << "Processo terminato con codice:" << retCode;
        sCommand = "C:\\RAMOptimizer\\RAMMap.exe -Es";
        retCode = QProcess::execute(sCommand);
        Sleep(iIntervalBetweenCommands);
        qDebug() << "Processo terminato con codice:" << retCode;
        sCommand = "C:\\RAMOptimizer\\RAMMap.exe -Em";
        retCode = QProcess::execute(sCommand);
        Sleep(iIntervalBetweenCommands);
        qDebug() << "Processo terminato con codice:" << retCode;
        sCommand = "C:\\RAMOptimizer\\RAMMap.exe -Et";
        retCode = QProcess::execute(sCommand);
        Sleep(iIntervalBetweenCommands);
        qDebug() << "Processo terminato con codice:" << retCode;
        sCommand = "C:\\RAMOptimizer\\RAMMap.exe -E0";
        retCode = QProcess::execute(sCommand);
        Sleep(iIntervalBetweenCommands);
        qDebug() << "Processo terminato con codice:" << retCode;
    }

    double getFreeRAM()
    {
        const double dBytesToMB = 1024.0 * 1024.0;
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
    readSettings();
    Worker worker;
    // QTimer timer;
    // Connect the timer timeout to the worker's job
    QObject::connect(&timer, SIGNAL(timeout()), &worker, SLOT(doWork()));
    // Start timer with 10 seconds interval (10000 ms)
    timer.start(iIntervalInSeconds * 1000);
    return app.exec();
}

#include "main.moc"
