#include <stdafx.h>

#include "memory.hpp"
#include "utils.hpp"

using namespace std::chrono_literals;

namespace {
    std::optional<Module> find_module(std::string_view module, DWORD pid) {
        wil::unique_handle snapshot{CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid)};
        if (!snapshot)
            return std::nullopt;

        MODULEENTRY32 mod = {0};
        mod.dwSize        = sizeof(mod);
        if (Module32First(snapshot.get(), &mod)) {
            do {
                if (!module.compare(mod.szModule)) {
                    return Module(mod);
                }
            } while (Module32Next(snapshot.get(), &mod));
        }
        return std::nullopt;
    }

    std::optional<DWORD> find_process(std::string_view name) {
        wil::unique_handle snapshot{CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL)};
        if (!snapshot)
            return std::nullopt;

        PROCESSENTRY32 entry = {0};
        entry.dwSize         = sizeof(entry);
        if (Process32First(snapshot.get(), &entry)) {
            do {
                if (utl::icompare(name, entry.szExeFile)) {
                    return entry.th32ProcessID;
                }
            } while (Process32Next(snapshot.get(), &entry));
        }

        return std::nullopt;
    }
} // namespace

std::optional<Memory> Memory::init(std::string_view proc, std::string_view window_name) {
    HWND window = FindWindowA(nullptr, std::string(window_name).c_str());
    if (!window) {
        return std::nullopt;
    }

    const auto pid = find_process(proc);
    if (!pid) {
        return std::nullopt;
    }

    wil::unique_process_handle process{OpenProcess(PROCESS_ALL_ACCESS, FALSE, *pid)};
    if (!process.is_valid()) {
        return std::nullopt;
    }

    while (true) {
        const auto engine = find_module(xorstr("engine.dll"), *pid),
                   client = find_module(xorstr("client.dll"), *pid);
        if (engine && client) {
            return Memory{
                .window     = window,
                .process    = std::move(process),
                .process_id = *pid,
                .client_dll = client.value(),
                .engine_dll = engine.value(),
            };
        }
        std::this_thread::sleep_for(2000ms);
    }
}
