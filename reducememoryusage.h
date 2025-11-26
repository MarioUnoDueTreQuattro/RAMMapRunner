#ifndef REDUCEMEMORYUSAGE_H
#define REDUCEMEMORYUSAGE_H

#include <QObject>
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>

class reduceMemoryUsage : public QObject
{
    Q_OBJECT
public:
    explicit reduceMemoryUsage(QObject* parent = nullptr);
    double getFreeRAM() ;
    void setAllProcessesWorkingSetSize(bool bCleanSystem=true);
    bool emptySystemWorkingSets(bool bCleanSystem=true);
    static bool enablePrivilege(LPCTSTR privilegeName);
    // Static wrapper for QtConcurrent
    static void runEmptySystemWorkingSets(reduceMemoryUsage *self);
signals:
    void reduceMemoryUsageFinished(bool success);
};

#endif // REDUCEMEMORYUSAGE_H
