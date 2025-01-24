#include <windows.h>
#include <stdio.h>
#include <tlhelp32.h>
#include <stdlib.h>  // For dynamic memory management

// Function prototypes
void TerminateTargetProcesses(DWORD* targetPIDs, DWORD count, DWORD delay);
DWORD* GetProcessIdsByName(const char* processName, DWORD* count);

int main() {
    const char* monitoredProcessName = "nikke.exe";
    const char* targetProcessName = "tbs_browser.exe";
    DWORD delay = 5000;          // 5-second delay before termination
    DWORD checkInterval = 15000; // Check every 15 seconds

    while (1) {
        DWORD monitoredCount = 0;
        DWORD* monitoredPIDs = GetProcessIdsByName(monitoredProcessName, &monitoredCount);

        if (monitoredCount == 0) {
            printf("Monitored process (%s) not found. Checking for targets...\n", monitoredProcessName);
            
            DWORD targetCount = 0;
            DWORD* targetPIDs = GetProcessIdsByName(targetProcessName, &targetCount);
            
            if (targetCount > 0) {
                TerminateTargetProcesses(targetPIDs, targetCount, delay);
                free(targetPIDs);
            } else {
                printf("No target processes (%s) found.\n", targetProcessName);
            }
        } else {
            printf("Monitoring process %s (PID: %d)\n", monitoredProcessName, monitoredPIDs[0]);
            free(monitoredPIDs);
        }

        Sleep(checkInterval);
    }

    return 0;
}

// Terminate multiple target processes after a delay
void TerminateTargetProcesses(DWORD* targetPIDs, DWORD count, DWORD delay) {
    Sleep(delay); // Wait before terminating all

    for (DWORD i = 0; i < count; i++) {
        HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, targetPIDs[i]);
        if (hProcess != NULL) {
            if (TerminateProcess(hProcess, 0)) {
                printf("Terminated target process (PID: %d)\n", targetPIDs[i]);
            } else {
                printf("Failed to terminate PID %d (Error: %d)\n", targetPIDs[i], GetLastError());
            }
            CloseHandle(hProcess);
        } else {
            printf("Could not open handle for PID %d (Error: %d)\n", targetPIDs[i], GetLastError());
        }
    }
}

// Get ALL PIDs matching a process name (returns dynamically allocated array)
DWORD* GetProcessIdsByName(const char* processName, DWORD* count) {
    DWORD* pids = NULL;
    *count = 0;
    
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return NULL;

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnapshot, &pe32)) {
        do {
            if (strcmp(pe32.szExeFile, processName) == 0) {
                // Dynamically resize the array
                DWORD* temp = realloc(pids, (*count + 1) * sizeof(DWORD));
                if (!temp) {
                    free(pids);
                    *count = 0;
                    CloseHandle(hSnapshot);
                    return NULL;
                }
                pids = temp;
                pids[(*count)++] = pe32.th32ProcessID;
            }
        } while (Process32Next(hSnapshot, &pe32));
    }

    CloseHandle(hSnapshot);
    return pids;
}