#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <string>



// Get process id by name

DWORD GetProcessID(const std::string& processName) {
    PROCESSENTRY32 processEntry;
    processEntry.dwSize = sizeof(PROCESSENTRY32);
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (snapshot == INVALID_HANDLE_VALUE) {
        return 0;
    }

    if (Process32First(snapshot, &processEntry)) {
        do {
            if (processName == processEntry.szExeFile) {
                CloseHandle(snapshot);
                return processEntry.th32ProcessID;
            }
        } while (Process32Next(snapshot, &processEntry));
    }

    CloseHandle(snapshot);
    return 0;
}


// ******************************************************


// Read memory

bool ReadMemory(DWORD processID, LPCVOID address, LPVOID buffer, SIZE_T size) {
    HANDLE hProcess = OpenProcess(PROCESS_VM_READ, FALSE, processID);
    if (!hProcess) {
        std::cerr << "Failed to open process for reading." << std::endl;
        return false;
    }

    BOOL result = ReadProcessMemory(hProcess, address, buffer, size, nullptr);
    CloseHandle(hProcess);
    return result;
}



// ******************************************************


// Get module base address

uintptr_t GetModuleBaseAddress(DWORD processID, const std::string& moduleName) {
    MODULEENTRY32 moduleEntry;
    moduleEntry.dwSize = sizeof(MODULEENTRY32);
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processID);

    if (snapshot == INVALID_HANDLE_VALUE) {
        return 0;
    }

    if (Module32First(snapshot, &moduleEntry)) {
        do {
            if (moduleName == moduleEntry.szModule) {
                CloseHandle(snapshot);
                return reinterpret_cast<uintptr_t>(moduleEntry.modBaseAddr);
            }
        } while (Module32Next(snapshot, &moduleEntry));
    }

    CloseHandle(snapshot);
    return 0;
}


//*******************************************************************************





// Write memory


bool WriteMemory(DWORD processID, LPVOID address, LPCVOID buffer, SIZE_T size) {
    HANDLE hProcess = OpenProcess(PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, processID);
    if (!hProcess) {
        std::cerr << "Failed to open process for writing." << std::endl;
        return false;
    }

    BOOL result = WriteProcessMemory(hProcess, address, buffer, size, nullptr);
    CloseHandle(hProcess);
    return result;
}


//********************************************************


void ClickMouse() {
    INPUT input = { 0 };
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    SendInput(1, &input, sizeof(INPUT));

    ZeroMemory(&input, sizeof(INPUT));
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
    SendInput(1, &input, sizeof(INPUT));
}



// Main function

int main() {
    std::string processName = "cs2.exe";
    std::string moduleName = "client.dll"; 

    DWORD processID = GetProcessID(processName);
    std::cout << "Process ID: " << processID << std::endl;
    if (!processID) {
        std::cerr << "Process not found." << std::endl;
        return 1;
    }

    uintptr_t baseAddress = GetModuleBaseAddress(processID, moduleName);
    std::cout << "Module base address: " << "0x" << baseAddress << std::endl;
    if (!baseAddress) {
        std::cerr << "Failed to get module base address." << std::endl;
        return 1;
    }

    uintptr_t offset1 = 0x1824A18;  // first offset
    uintptr_t offset2 = 0x13A8;  // second offset

    uintptr_t firstPointerAddress = baseAddress + offset1;
    uintptr_t actualAddress;

    // Read the first pointer
    if (!ReadMemory(processID, reinterpret_cast<LPCVOID>(firstPointerAddress), &actualAddress, sizeof(actualAddress))) {
        std::cerr << "Failed to read first pointer." << std::endl;
        return 1;
    }

    uintptr_t entityIDAddress = actualAddress + offset2;

    bool triggerbotEnabled = false;

    std::cout << "Press LEFT ALT to toggle triggerbot on/off." << std::endl;

    while (true) {
        // Check if Shift key is pressed
        if (GetAsyncKeyState(VK_MENU) & 0x8000) {
            triggerbotEnabled = !triggerbotEnabled;
            std::cout << "Triggerbot " << (triggerbotEnabled ? "enabled" : "disabled") << std::endl;
            Sleep(300);  // Debounce to prevent multiple toggles from a single press
        }

        if (triggerbotEnabled) {
            int entityID;
            if (ReadMemory(processID, reinterpret_cast<LPCVOID>(entityIDAddress), &entityID, sizeof(entityID))) {
                if (entityID >= 0) {
                    ClickMouse();
                }
            }
            else {
                std::cerr << "Failed to read entityID." << std::endl;
            }
        }

        Sleep(10);  // Delay to prevent CPU overload
    }

    return 0;
}

