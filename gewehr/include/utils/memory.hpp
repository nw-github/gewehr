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

    DWORD_PTR get_image_base() { return m_addr; }
    DWORD_PTR get_image_size() { return m_size; }

private:
    DWORD_PTR m_addr;
    DWORD_PTR m_size;

};

class Memory
{
public:
    bool attach(std::string_view proc, std::string_view window);
    bool find_modules();
    void detach();

    bool read(DWORD_PTR address, LPVOID buffer, DWORD size);
    bool write(DWORD_PTR address, LPCVOID buffer, DWORD size);

    template<typename T>
    T read(DWORD_PTR address)
    {
        T val;
        read(address, &val, sizeof(T));
        return val;
    }

    template<typename T>
    bool write(DWORD_PTR address, const T &value)
    {
        return write(address, &value, sizeof(T));
    }

    template<typename T>
    bool write_protected(DWORD_PTR address, const T& value)
    {
        DWORD old_protect;
        VirtualProtectEx(process, (LPVOID)address, sizeof(T), PAGE_EXECUTE_READWRITE, &old_protect);
        bool result = write(address, value);
        VirtualProtectEx(process, (LPVOID)address, sizeof(T), old_protect, nullptr);
        return result;
    }

public:
    Module        engine_dll;
    Module        client_dll;

    wil::unique_handle process;
    DWORD        process_id{0};
    HWND        window{nullptr};

private:
    bool find_module(std::string_view module, Module &out_module);

};

namespace g
{
    inline std::unique_ptr<Memory> memory = std::make_unique<Memory>();
}