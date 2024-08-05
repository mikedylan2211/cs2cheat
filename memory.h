#pragma once
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


// click left mouse button

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

