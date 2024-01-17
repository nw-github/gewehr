#include <stdafx.h>

#include "utils/memory.hpp"

using namespace std::chrono_literals;

namespace {
    std::optional<Module> find_module(std::string_view module, DWORD process_id) {
        wil::unique_handle hSnapshot{ CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, process_id) };
        if (!hSnapshot)
            return std::nullopt;

        MODULEENTRY32 mod = { 0 };
        mod.dwSize = sizeof(MODULEENTRY32);
        if (Module32First(hSnapshot.get(), &mod)) {
            do {
                if (!module.compare(mod.szModule)) {
                    return Module(mod);
                }
            } while (Module32Next(hSnapshot.get(), &mod));
        }
        return std::nullopt;
    }
}

std::optional<Memory> Memory::init(std::string_view proc, std::string_view window_name)
{
    HWND window = FindWindowA(nullptr, std::string(window_name).c_str());
    if (!window)
        return std::nullopt;

    wil::unique_handle snapshot{CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL)};
    if (!snapshot)
        return std::nullopt;

    PROCESSENTRY32 entry = {0};
    entry.dwSize = sizeof(PROCESSENTRY32);

    DWORD process_id = 0;
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
        return std::nullopt;

    wil::unique_process_handle process{ OpenProcess(PROCESS_ALL_ACCESS, FALSE, process_id) };
    if (!process.is_valid()) {
        return std::nullopt;
    }

    while (true) {
        std::this_thread::sleep_for(2000ms);
        const auto engine = find_module(xorstr("engine.dll"), process_id),
            client = find_module(xorstr("client.dll"), process_id);
        if (engine && client) {
            return Memory{
                .window = window,
                .process = std::move(process),
                .process_id = process_id,
                .client_dll = client.value(),
                .engine_dll = engine.value(),
            };
        }
    }
}

bool Memory::read(DWORD_PTR address, LPVOID buffer, DWORD size) const
{
    return ReadProcessMemory(process.get(), (LPCVOID)address, buffer, size, nullptr);
}

bool Memory::write(DWORD_PTR address, LPCVOID buffer, DWORD size) const
{
    return WriteProcessMemory(process.get(), (LPVOID)address, buffer, size, nullptr);
}
