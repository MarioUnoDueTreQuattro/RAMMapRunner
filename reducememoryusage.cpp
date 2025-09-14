#include "reducememoryusage.h"
#include <QDebug>
#include <iostream>

reduceMemoryUsage::reduceMemoryUsage(QObject *parent)
{

}

typedef enum _SYSTEM_MEMORY_LIST_COMMAND
{
    MemoryCaptureAccessedBits = 0,
    MemoryCaptureAndResetAccessedBits = 1,
    MemoryEmptyWorkingSets = 2,
    MemoryFlushModifiedList = 3,
    MemoryPurgeStandbyList = 4,
    MemoryPurgeLowPriorityStandbyList = 5,
    MemoryCommandMax                      // Not a command, just a count
} SYSTEM_MEMORY_LIST_COMMAND;

typedef NTSTATUS (NTAPI *PFN_NtSetSystemInformation)(
    ULONG SystemInformationClass,
    PVOID SystemInformation,
    ULONG SystemInformationLength
);

#define SystemMemoryListInformation 0x50  // undocumented

bool reduceMemoryUsage::enablePrivilege(LPCTSTR privilegeName)
{
    HANDLE token = NULL;
    if (!OpenProcessToken(GetCurrentProcess(),
        TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
        &token))
    {
        qDebug() << "OpenProcessToken failed:" << GetLastError();
        return false;
    }
    TOKEN_PRIVILEGES tp;
    LUID luid;
    if (!LookupPrivilegeValue(NULL, privilegeName, &luid))
    {
        qDebug() << "LookupPrivilegeValue failed:" << GetLastError();
        CloseHandle(token);
        return false;
    }
    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    if (!AdjustTokenPrivileges(token, FALSE, &tp, sizeof(tp), NULL, NULL))
    {
        qDebug() << "AdjustTokenPrivileges call failed:" << GetLastError();
        CloseHandle(token);
        return false;
    }
    DWORD err = GetLastError();
    CloseHandle(token);
    if (err == ERROR_NOT_ALL_ASSIGNED)
    {
        qDebug() << "Privilege" << privilegeName << "not held by this process token!";
        return false;
    }
    //qDebug() << "Privilege" << privilegeName << "enabled successfully.";
    return true;
}

void reduceMemoryUsage::setAllProcessesWorkingSetSize()
{
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE)
    {
        // Handle error, e.g., show a message box
        qDebug() << "error: hProcessSnap == INVALID_HANDLE_VALUE";
        return;
    }
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    if (Process32First(hProcessSnap, &pe32))
    {
        do
        {
            // Convert the wide-character string (wchar_t*) to QString
            QString processName = QString::fromWCharArray(pe32.szExeFile);
            // qDebug() << __FUNCTION__ << processName;
            DWORD processId = pe32.th32ProcessID;
            if (processId == 0 );//|| processId == 4) continue; // Skip Idle and System
            HANDLE hProcess = OpenProcess(PROCESS_SET_QUOTA | PROCESS_QUERY_INFORMATION, FALSE, processId);
            if (hProcess != nullptr)
            {
                // WINBOOL bSuccess = SetProcessWorkingSetSize(hProcess, (SIZE_T) -1, (SIZE_T) -1);
                WINBOOL bSuccess = EmptyWorkingSet(hProcess);
                if (bSuccess)
                {
                    SetProcessWorkingSetSizeEx(hProcess, (SIZE_T) -1, (SIZE_T) -1, QUOTA_LIMITS_HARDWS_MIN_ENABLE | QUOTA_LIMITS_HARDWS_MAX_ENABLE);
                    //LOG_MSG(processName + " PID " + QString::number (processId) + " SetProcessWorkingSetSizeEx");
                }
                else
                {
                    std::cout << processName.toStdString () << " " << "FAILED to trim working set. Error: " << GetLastError() << "\n";
                }
                CloseHandle(hProcess);
            }
        }
        while (Process32Next(hProcessSnap, &pe32));
        emptySystemWorkingSets();
    }
    CloseHandle(hProcessSnap);
}

bool reduceMemoryUsage::emptySystemWorkingSets()
{
    bool result = false;
    // Must enable BOTH privileges
    bool ok1 = enablePrivilege(SE_INCREASE_QUOTA_NAME);
    bool ok2 = enablePrivilege(SE_PROF_SINGLE_PROCESS_NAME);
    if (!ok1 || !ok2)
    {
        qDebug() << "Failed to enable required privileges. Run as Administrator?";
        emit reduceMemoryUsageFinished(false);
        return false;
    }
    HMODULE ntdll = GetModuleHandle(TEXT("ntdll.dll"));
    if (!ntdll)
    {
        qDebug() << "Cannot load ntdll.dll";
        emit reduceMemoryUsageFinished(false);
        return false;
    }
    PFN_NtSetSystemInformation pNtSetSystemInformation =
        (PFN_NtSetSystemInformation)GetProcAddress(ntdll, "NtSetSystemInformation");
    if (!pNtSetSystemInformation)
    {
        qDebug() << "Cannot resolve NtSetSystemInformation";
        emit reduceMemoryUsageFinished(false);
        return false;
    }
    SYSTEM_MEMORY_LIST_COMMAND command;
    NTSTATUS status;
    command = MemoryFlushModifiedList;
    status = pNtSetSystemInformation(
            SystemMemoryListInformation,
            &command,
            sizeof(command)
        );
    if (status != 0) qDebug() << "NtSetSystemInformation (MemoryFlushModifiedList) failed. NTSTATUS:" << hex << status;
    command = MemoryEmptyWorkingSets;
    status = pNtSetSystemInformation(
            SystemMemoryListInformation,
            &command,
            sizeof(command)
        );
    if (status != 0) qDebug() << "NtSetSystemInformation (MemoryEmptyWorkingSets) failed. NTSTATUS:" << hex << status;
    command = MemoryPurgeStandbyList;
    status = pNtSetSystemInformation(
            SystemMemoryListInformation,
            &command,
            sizeof(command)
        );
    if (status != 0) qDebug() << "NtSetSystemInformation (MemoryPurgeStandbyList) failed. NTSTATUS:" << hex << status;
    command = MemoryPurgeLowPriorityStandbyList;
    status = pNtSetSystemInformation(
            SystemMemoryListInformation,
            &command,
            sizeof(command)
        );
    if (status != 0) qDebug() << "NtSetSystemInformation (MemoryPurgeLowPriorityStandbyList) failed. NTSTATUS:" << hex << status;
    command = MemoryCaptureAndResetAccessedBits;
    status = pNtSetSystemInformation(
            SystemMemoryListInformation,
            &command,
            sizeof(command)
        );
    if (status != 0) qDebug() << "NtSetSystemInformation (MemoryCaptureAndResetAccessedBits) failed. NTSTATUS:" << hex << status;
    // Open the "System" process (PID 4 on most systems)
    HANDLE hSystem = OpenProcess(PROCESS_SET_QUOTA | PROCESS_QUERY_INFORMATION,
            FALSE, 4);
    if (hSystem)
    {
        // Trim PID 4 working set (system WS)
        SetProcessWorkingSetSize(hSystem, (SIZE_T) -1, (SIZE_T) -1);
        CloseHandle(hSystem);
    }
    if (status != 0)
    {
        qDebug() << "NtSetSystemInformation failed. NTSTATUS:" << QString("0x%1").arg(status, 8, 16, QLatin1Char('0')).toUpper();
        return false;
    }
    //LOG_MSG("Memory usage reduced successfully.");
    result = (status == 0); // NTSTATUS 0 = STATUS_SUCCESS
    emit reduceMemoryUsageFinished(result);
    return result;
}

void reduceMemoryUsage::runEmptySystemWorkingSets(reduceMemoryUsage *self)
{
    if (self)
        self->emptySystemWorkingSets();
}

double reduceMemoryUsage::getFreeRAM()
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
