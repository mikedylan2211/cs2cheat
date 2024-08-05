#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <string>
#include "memory.h"


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

    uintptr_t dwLocalPlayerPawn = 0x1824A18;  // first offset
    uintptr_t m_iIDEntIndex = 0x13A8;  // second offset

    uintptr_t firstPointerAddress = baseAddress + dwLocalPlayerPawn;
    uintptr_t actualAddress;

    // Read the first pointer
    if (!ReadMemory(processID, reinterpret_cast<LPCVOID>(firstPointerAddress), &actualAddress, sizeof(actualAddress))) {
        std::cerr << "Failed to read first pointer." << std::endl;
        return 1;
    }

    uintptr_t entityIDAddress = actualAddress + m_iIDEntIndex;

    bool triggerbotEnabled = false;

    std::cout << "Press LEFT ALT to toggle triggerbot on/off." << std::endl;

    while (true) {
        // Check if alt key is pressed
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

