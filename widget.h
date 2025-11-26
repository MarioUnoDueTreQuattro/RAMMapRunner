#ifndef WIDGET_H
#define WIDGET_H

#include <QApplication>
#include <QCoreApplication>
#include <QWidget>
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
#include "reducememoryusage.h"
#include "osdwidget.h"

//// Include Windows API headers
//#ifdef Q_OS_WIN
// #include <windows.h>
//    //    #include <psapi.h> // For EnumProcesses, EnumProcessModules, GetModuleBaseName
//#endif

//void readSettings()
//{
// qApp->setOrganizationName("andreag");
// qApp->setApplicationName("RAMMapRunner");
// QSettings settings;
// bool bSettingExists;
// bSettingExists = settings.contains ("bUseInternalClean");
// if (!bSettingExists) settings.setValue ("bUseInternalClean", true);
// bUseInternalClean = settings.value ("bUseInternalClean", true).toBool ();
// bSettingExists = settings.contains ("iIntervalInSeconds");
// if (!bSettingExists) settings.setValue ("iIntervalInSeconds", 30);
// iIntervalInSeconds = settings.value ("iIntervalInSeconds", 10).toInt ();
// bSettingExists = settings.contains ("iMinimumIntervalInSeconds");
// if (!bSettingExists) settings.setValue ("iMinimumIntervalInSeconds", 30);
// iMinimumIntervalInSeconds = settings.value ("iMinimumIntervalInSeconds", 10).toInt ();
// bSettingExists = settings.contains ("iIntervalBetweenCommands");
// if (!bSettingExists) settings.setValue ("iIntervalBetweenCommands", 1000);
// iIntervalBetweenCommands = settings.value ("iIntervalBetweenCommands", 1000).toInt ();
// bSettingExists = settings.contains ("iCounterThreshold");
// if (!bSettingExists) settings.setValue ("iCounterThreshold", 10);
// iCounterThreshold = settings.value ("iCounterThreshold", 10).toInt ();
// bSettingExists = settings.contains ("iMemoryLimit");
// if (!bSettingExists) settings.setValue ("iMemoryLimit", 1280);
// iMemoryLimit = settings.value ("iMemoryLimit", 1280).toInt ();
// bSettingExists = settings.contains ("iLowMemoryMemoryLimit");
// if (!bSettingExists) settings.setValue ("iLowMemoryMemoryLimit", 1024);
// iLowMemoryMemoryLimit = settings.value ("iLowMemoryMemoryLimit", 1024).toInt ();
// bSettingExists = settings.contains ("iLowMemoryCheckIntervalInSeconds");
// if (!bSettingExists) settings.setValue ("iLowMemoryCheckIntervalInSeconds", 10);
// iLowMemoryCheckIntervalInSeconds = settings.value ("iLowMemoryCheckIntervalInSeconds", 10).toInt ();
//    // logger.write("iIntervalInSeconds " +QString::number (iIntervalInSeconds));
//    // logger.write("iIntervalBetweenCommands " +QString::number (iIntervalBetweenCommands));
//    // logger.write("iCounterThreshold " +QString::number (iCounterThreshold));
//    // logger.write("iMemoryLimit " +QString::number (iMemoryLimit));
//}

namespace Ui
{
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();
protected:
    bool nativeEvent(const QByteArray &eventType, void *message, long *result) override;

private:
    OSDWidget *osd;
    Ui::Widget *ui;
    static const int HOTKEY_ID = 1; // Identifier for our hotkey
    static const int HOTKEY2_ID = 2; // Identifier for our hotkey
    bool registerGlobalHotkey();
    void unregisterGlobalHotkey();
    void readSettings();
    bool bOSD_Enabled;
    int iTextSize;
    int iDuration;
    QString sPosition;
    bool bUseInternalClean;
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
    Logger logger;//("c:\\RAMMapRunner.log");
    const double dBytesToMB = 1024.0 * 1024.0;
    int runCommands(bool bCleanSystem=true);
    void doMemoryWork();
    void doLowMemoryWork();
    double getFreeRAM();
private slots:
    void doWork();

};

#endif // WIDGET_H
