#pragma once

class Module
{
public:
    Module()
        : m_addr{0}, m_size{0}
    { }

    Module(const MODULEENTRY32 &entry)
        : m_addr{reinterpret_cast<DWORD_PTR>(entry.modBaseAddr)}, m_size{entry.modBaseSize}
    { }

    DWORD_PTR get_image_base() const { return m_addr; }
    DWORD_PTR get_image_size() const { return m_size; }

private:
    DWORD_PTR m_addr;
    DWORD_PTR m_size;

};

class Memory
{
public:
    static std::optional<Memory> init(std::string_view proc, std::string_view window);

    bool read(DWORD_PTR address, LPVOID buffer, DWORD size) const;
    bool write(DWORD_PTR address, LPCVOID buffer, DWORD size) const;

    template<typename T>
    T read(DWORD_PTR address) const {
        T val;
        read(address, &val, sizeof(T));
        return val;
    }

    template<typename T>
    bool write(DWORD_PTR address, const T &value) const {
        return write(address, &value, sizeof(T));
    }

    template<typename T>
    bool write_protected(DWORD_PTR address, const T& value) const {
        DWORD old_protect;
        VirtualProtectEx(process, (LPVOID)address, sizeof(T), PAGE_EXECUTE_READWRITE, &old_protect);
        bool result = write(address, value);
        VirtualProtectEx(process, (LPVOID)address, sizeof(T), old_protect, nullptr);
        return result;
    }

public:
    HWND        window{nullptr};
    wil::unique_process_handle process;
    DWORD        process_id{0};
    Module        client_dll;
    Module        engine_dll;

};
