#pragma once

struct Module {
public:
    DWORD_PTR base_addr;
    DWORD_PTR size;

    Module(const MODULEENTRY32 &entry)
        : base_addr{reinterpret_cast<DWORD_PTR>(entry.modBaseAddr)}, size{entry.modBaseSize}
    { }
};

class Memory {
public:
    static std::optional<Memory> init(std::string_view proc, std::string_view window);

    bool read(DWORD_PTR addr, LPVOID buffer, DWORD size) const {
        return ReadProcessMemory(process.get(), (LPCVOID)addr, buffer, size, nullptr);
    }

    bool write(DWORD_PTR addr, LPCVOID buffer, DWORD size) const {
        return WriteProcessMemory(process.get(), (LPVOID)addr, buffer, size, nullptr);
    }

    template<typename T>
    T read(DWORD_PTR addr) const {
        T val;
        read(addr, &val, sizeof(val));
        return val;
    }

    template<typename T>
    bool write(DWORD_PTR addr, const T &val) const {
        return write(addr, &val, sizeof(val));
    }

    template<typename T>
    bool write_protected(DWORD_PTR addr, const T& val) const {
        DWORD old_protect;
        VirtualProtectEx(process, (LPVOID)addr, sizeof(val), PAGE_EXECUTE_READWRITE, &old_protect);
        bool result = write(addr, val);
        VirtualProtectEx(process, (LPVOID)addr, sizeof(val), old_protect, nullptr);
        return result;
    }

public:
    HWND window{nullptr};
    wil::unique_process_handle process;
    DWORD process_id{0};
    Module client_dll;
    Module engine_dll;

};
