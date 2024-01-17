#include <stdafx.h>

#include "utils/memory.hpp"

bool Memory::attach(std::string_view proc, std::string_view window_name)
{
    detach();

    window = FindWindowA(nullptr, std::string(window_name).c_str());
    if (!window)
        return false;

    wil::unique_handle snapshot{CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL)};
    if (!snapshot)
        return false;

    PROCESSENTRY32 entry = {0};
    entry.dwSize = sizeof(PROCESSENTRY32);
    if (Process32First(snapshot.get(), &entry))
    {
        do
        {
            if (!proc.compare(entry.szExeFile))
            {
                process_id = entry.th32ProcessID;
                break;
            }
        } while (Process32Next(snapshot.get(), &entry));
    }

    if (process_id == 0)
        return false;

    process.reset(OpenProcess(PROCESS_ALL_ACCESS, FALSE, process_id));
    return process.is_valid();
}

bool Memory::find_modules()
{
    if (!find_module(xorstr("engine.dll"), engine_dll))
        return false;
    if (!find_module(xorstr("client.dll"), client_dll))
        return false;

    return true;
}

void Memory::detach()
{
    process.reset(nullptr);
    process_id = 0;
    window = nullptr;
    client_dll = Module{};
    engine_dll = Module{};
}

bool Memory::find_module(std::string_view module, Module &out_module)
{
    wil::unique_handle hSnapshot{CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, process_id)};
    if (!hSnapshot)
        return false;

    MODULEENTRY32 mod = {0};
    mod.dwSize = sizeof(MODULEENTRY32);
    if (Module32First(hSnapshot.get(), &mod)) {
        do {
            if (!module.compare(mod.szModule)) {
                out_module = Module(mod);
                return true;
            }
        } while (Module32Next(hSnapshot.get(), &mod));
    }
    return false;
}

bool Memory::read(DWORD_PTR address, LPVOID buffer, DWORD size)
{
    return ReadProcessMemory(process.get(), (LPCVOID)address, buffer, size, nullptr);
}

bool Memory::write(DWORD_PTR address, LPCVOID buffer, DWORD size)
{
    return WriteProcessMemory(process.get(), (LPVOID)address, buffer, size, nullptr);
}