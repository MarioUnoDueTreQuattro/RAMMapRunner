#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget), logger("c:\\RAMMapRunner.log")
{
    ui->setupUi(this);
    if (registerGlobalHotkey())
        qDebug() << "Global hotkey \"Alt-Win-F\" registered successfully.";
    else
        qDebug() << "Failed to register \"Alt-Win-F\" global hotkey.";
    logger.write("Application started");
    osd = new OSDWidget();
    // Optional: make the OSD click-through so it doesn't intercept mouse.
    osd->setClickThrough(true);
    readSettings();
    QObject::connect(&checkLowMemoryTimer, SIGNAL(timeout()), this, SLOT(doWork()));
    checkLowMemoryTimer.start(iLowMemoryCheckIntervalInSeconds * 1000);
    elapsedTimer.start();
    this->hide();
}

Widget::~Widget()
{
    // delete timer;
    delete osd;
    osd = nullptr;
    unregisterGlobalHotkey();
    delete ui;
}

int Widget::runCommands(bool bCleanSystem)
{
    QString sMessage;
    if (bUseInternalClean)
    {
        reduceMemoryUsage rmu;
        rmu.setAllProcessesWorkingSetSize(bCleanSystem);
        dFreeMem = getFreeRAM();
        sMessage = "Free RAM after cleanup: " + QString::number(dFreeMem) + " MB";
        logger.write(sMessage);
        qDebug() << sMessage;
        sMessage = "Free RAM: " + QString::number(qRound(dFreeMem)) + " MB";
        osd->showMessage(sMessage);
        return 0;
    }
    else
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
        sMessage = "Free RAM after cleanup: " + QString::number(dFreeMem) + " MB";
        logger.write(sMessage);
        qDebug() << sMessage;
        sMessage = "Free RAM: " + QString::number(qRound(dFreeMem)) + " MB";
        osd->showMessage(sMessage);
        return retCode;
    }
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

void Widget::doWork()
{
    // qint64 elapsedTimeMs = elapsedTimer.elapsed();
    // double elapsedTimeSec = static_cast<double>(elapsedTimeMs) / 1000.0;
    // if (elapsedTimeSec < iMinimumIntervalInSeconds) return;
    readSettings();
    //timer.setInterval (iIntervalInSeconds * 1000);
    checkLowMemoryTimer.setInterval(iLowMemoryCheckIntervalInSeconds * 1000);
    dFreeMem = getFreeRAM();
    qDebug() << "Free RAM: " << dFreeMem;
    // logger.write("Free RAM: " + QString::number (dFreeMem));
    iCounter++;
    if (dFreeMem < iMemoryLimit && iCounter >= iCounterThreshold)
    {
        qDebug() << "iCounter: " << iCounter;
        // QString sMessage = "Low RAM: " + QString::number(qRound(dFreeMem)) + " MB";
        // osd->showMessage (sMessage);
        doMemoryWork();
        // return;
    }
    else if (dFreeMem < iLowMemoryMemoryLimit)
    {
        qDebug() << "iCounter: " << iCounter;
        // QString sMessage = "Very low RAM: " + QString::number(qRound(dFreeMem)) + " MB";
        // osd->showMessage (sMessage);
        doLowMemoryWork();
        // return;
    }
    //elapsedTimer.start ();
}

void Widget::doMemoryWork()
{
    // qint64 elapsedTimeMs = elapsedTimer.elapsed();
    // double elapsedTimeSec = static_cast<double>(elapsedTimeMs) / 1000.0;
    // if (elapsedTimeSec < iMinimumIntervalInSeconds) return;
    iCounter = 0;
    qDebug() << "doMemoryWork Job started at:" << QDateTime::currentDateTime().toString();
    logger.write("Free RAM: " + QString::number(dFreeMem));
    logger.write("doMemoryWork Job started");
    int retCode = runCommands(false);
    qDebug() << "Processo terminato con codice:" << retCode;
}

void Widget::doLowMemoryWork()
{
    qint64 elapsedTimeMs = elapsedTimer.elapsed();
    double elapsedTimeSec = static_cast<double>(elapsedTimeMs) / 1000.0;
    if (elapsedTimeSec < iMinimumIntervalInSeconds) return;
    // readSettings();
    //elapsedTimer.stop ();
    iCounter = 0;
    qDebug() << "doLowMemoryWork Job started at:" << QDateTime::currentDateTime().toString();
    logger.write("Free RAM: " + QString::number(dFreeMem));
    logger.write("doLowMemoryWork Job started");
    int retCode = runCommands();
    qDebug() << "Processo terminato con codice:" << retCode;
    elapsedTimer.start();
}

double Widget::getFreeRAM()
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

void Widget::readSettings()
{
    qApp->setOrganizationName("andreag");
    qApp->setApplicationName("RAMMapRunner");
    QSettings settings;
    bool bSettingExists;
    bSettingExists = settings.contains("bUseInternalClean");
    if (!bSettingExists) settings.setValue("bUseInternalClean", true);
    bUseInternalClean = settings.value("bUseInternalClean", true).toBool();
    bSettingExists = settings.contains("iIntervalInSeconds");
    if (!bSettingExists) settings.setValue("iIntervalInSeconds", 30);
    iIntervalInSeconds = settings.value("iIntervalInSeconds", 10).toInt();
    bSettingExists = settings.contains("iMinimumIntervalInSeconds");
    if (!bSettingExists) settings.setValue("iMinimumIntervalInSeconds", 30);
    iMinimumIntervalInSeconds = settings.value("iMinimumIntervalInSeconds", 10).toInt();
    bSettingExists = settings.contains("iIntervalBetweenCommands");
    if (!bSettingExists) settings.setValue("iIntervalBetweenCommands", 1000);
    iIntervalBetweenCommands = settings.value("iIntervalBetweenCommands", 1000).toInt();
    bSettingExists = settings.contains("iCounterThreshold");
    if (!bSettingExists) settings.setValue("iCounterThreshold", 10);
    iCounterThreshold = settings.value("iCounterThreshold", 10).toInt();
    bSettingExists = settings.contains("iMemoryLimit");
    if (!bSettingExists) settings.setValue("iMemoryLimit", 1280);
    iMemoryLimit = settings.value("iMemoryLimit", 1280).toInt();
    bSettingExists = settings.contains("iLowMemoryMemoryLimit");
    if (!bSettingExists) settings.setValue("iLowMemoryMemoryLimit", 1024);
    iLowMemoryMemoryLimit = settings.value("iLowMemoryMemoryLimit", 1024).toInt();
    bSettingExists = settings.contains("iLowMemoryCheckIntervalInSeconds");
    if (!bSettingExists) settings.setValue("iLowMemoryCheckIntervalInSeconds", 10);
    iLowMemoryCheckIntervalInSeconds = settings.value("iLowMemoryCheckIntervalInSeconds", 10).toInt();
    bOSD_Enabled = settings.value("OSD_Enabled", true).toBool();
    iTextSize = settings.value("OSD_TextSize", 16).toInt();
    iDuration = settings.value("OSD_Duration", 2000).toInt();
    sPosition = settings.value("OSD_Position", "Center").toString();
    if (bOSD_Enabled)
    {
        osd->setTextSize(iTextSize);
        osd->setDuration(iDuration);
        osd->setPosition(sPosition);
    }
    // logger.write("iIntervalInSeconds " +QString::number (iIntervalInSeconds));
    // logger.write("iIntervalBetweenCommands " +QString::number (iIntervalBetweenCommands));
    // logger.write("iCounterThreshold " +QString::number (iCounterThreshold));
    // logger.write("iMemoryLimit " +QString::number (iMemoryLimit));
}

bool Widget::registerGlobalHotkey()
{
#define MOD_WIN 0x0008
    HWND hwnd = reinterpret_cast<HWND>(this->winId());
    // UINT modifiers = MOD_CONTROL | MOD_ALT;
    // UINT vk = 0x4D; // 'M'
    UINT modifiers = MOD_ALT | MOD_WIN;
    UINT vk = 0x46;
    BOOL ok = RegisterHotKey(hwnd, HOTKEY_ID, modifiers, vk);
    vk = 0x52;
    ok = RegisterHotKey(hwnd, HOTKEY2_ID, modifiers, vk);
    return (ok != 0);
}

void Widget::unregisterGlobalHotkey()
{
    HWND hwnd = reinterpret_cast<HWND>(this->winId());
    UnregisterHotKey(hwnd, HOTKEY_ID);
    UnregisterHotKey(hwnd, HOTKEY2_ID);
}

bool Widget::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
    MSG *msg = reinterpret_cast<MSG *>(message);
    if (msg->message == WM_HOTKEY)
    {
        int id = (int)msg->wParam;
        if (id == HOTKEY_ID)
        {
            //qDebug() << "Global hotkey \"Alt-Win-F\" pressed!";
            runCommands(false);
            return true; // Event handled
        }
        if (id == HOTKEY2_ID)
        {
            //qDebug() << "Global hotkey \"Alt-Win-F\" pressed!";
            runCommands();
            return true; // Event handled
        }
    }
    return false; // Let Qt handle other events
}
