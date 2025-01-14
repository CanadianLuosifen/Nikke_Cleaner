#include <windows.h>
#include <stdio.h>
#include <tlhelp32.h>  // Include the necessary header for process enumeration

// Function prototypes
void TerminateTargetProcess(DWORD targetPID, DWORD delay);
DWORD GetProcessIdByName(const char* processName);

int main() {
    // Process names to monitor and terminate
    const char* monitoredProcessName = "nikke.exe";
    const char* targetProcessName = "tbs_browser.exe";

    // Time delay in milliseconds before terminating the target process
    DWORD delay = 5000; // 5 seconds
    DWORD checkInterval = 15000; // 15 seconds

    while (1) { // Infinite loop to continuously check the process
        // Get the PIDs of the processes
        DWORD monitoredPID = GetProcessIdByName(monitoredProcessName);
        DWORD targetPID = GetProcessIdByName(targetProcessName);

        if (monitoredPID == 0) {
            printf("Error: Could not find the monitored process (%s)\n", monitoredProcessName);
            if (targetPID != 0) {
                // If the monitored process is not found but the target process is running, terminate the target process
                TerminateTargetProcess(targetPID, delay);
            }
        } else {
            printf("Monitoring process %s (PID: %d)\n", monitoredProcessName, monitoredPID);
            if (targetPID == 0) {
                printf("Error: Could not find the target process (%s)\n", targetProcessName);
            } else {
                printf("Target process %s (PID: %d) will be terminated after a delay of %d milliseconds if the monitored process ends.\n", targetProcessName, targetPID, delay);
            }
        }

        // Sleep for the specified interval before checking again
        Sleep(checkInterval);
    }

    return 0;
}

void TerminateTargetProcess(DWORD targetPID, DWORD delay) {
    // Sleep for the specified delay
    Sleep(delay);

    // Open a handle to the target process with necessary permissions
    HANDLE hTargetProcess = OpenProcess(PROCESS_TERMINATE, FALSE, targetPID);
    if (hTargetProcess == NULL) {
        printf("Error: Could not open handle to target process. Error code: %d\n", GetLastError());
        return;
    }

    // Terminate the target process
    if (TerminateProcess(hTargetProcess, 0)) {
        printf("Target process terminated successfully.\n");
    } else {
        printf("Error: Could not terminate target process. Error code: %d\n", GetLastError());
    }

    // Close the handle to the target process
    CloseHandle(hTargetProcess);
}

DWORD GetProcessIdByName(const char* processName) {
    DWORD pid = 0;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32);

        if (Process32First(hSnapshot, &pe32)) {
            do {
                if (strcmp(pe32.szExeFile, processName) == 0) {
                    pid = pe32.th32ProcessID;
                    break;
                }
            } while (Process32Next(hSnapshot, &pe32));
        }

        CloseHandle(hSnapshot);
    }

    return pid;
}