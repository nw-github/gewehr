#pragma once

class NetvarManager
{
public:
    DWORD scan_table(DWORD dwTableAddr, LPCSTR lpVarName, DWORD dwLevel);
    DWORD find_netvar(DWORD dwStart, LPCSTR lpClassName, LPCSTR lpVarName);

private:
    bool get_prop_name(DWORD dwAddress, PVOID pBuffer);
    bool get_table_name(DWORD dwAddress, PVOID pBuffer);

    int get_offset(DWORD dwAddress);
    int get_prop_count(DWORD dwAddress);

    DWORD get_data_table(DWORD dwAddress);
    DWORD get_prop_by_id(DWORD dwAddress, int iIndex);
    DWORD get_table(DWORD dwAddress);
    DWORD get_next_class(DWORD dwAddress);

};

namespace g
{
    inline std::unique_ptr<NetvarManager> netvars = std::make_unique<NetvarManager>();
}