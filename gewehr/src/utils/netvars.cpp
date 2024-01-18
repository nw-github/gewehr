#include <stdafx.h>

#include "utils/utils.hpp"

#include "utils/memory.hpp"

#define nCRecvPropSize 0x3C

// CRecvProp struct offsets
#define m_pVarName 0x0
#define m_pDataTable 0x28
#define m_iOffset 0x2C

// CRecvTable struct offsets
#define m_pProps 0x0
#define m_nProps 0x4
#define m_pNetTableName 0xC

// CClientClass struct offsets
#define m_pRecvTable 0xC
#define m_pNext 0x10

namespace
{

    /*
     ** Reversed Source SDK netvar classes
     */

    bool get_prop_name(const Memory& mem, DWORD dwAddress, PVOID pBuffer)
    {
        DWORD dwNameAddr = mem.read<DWORD>(dwAddress + m_pVarName);
        return mem.read(dwNameAddr, pBuffer, 128);
    }

    DWORD get_data_table(const Memory& mem, DWORD dwAddress)
    {
        return mem.read<DWORD>(dwAddress + m_pDataTable);
    }

    int get_offset(const Memory& mem, DWORD dwAddress)
    {
        return mem.read<int>(dwAddress + m_iOffset);
    }

    DWORD get_prop_by_id(const Memory& mem, DWORD dwAddress, int iIndex)
    {
        DWORD dwPropAddr = mem.read<DWORD>(dwAddress + m_pProps);
        return dwPropAddr + nCRecvPropSize * iIndex;
    }

    int get_prop_count(const Memory& mem, DWORD dwAddress)
    {
        return mem.read<int>(dwAddress + m_nProps);
    }

    bool get_table_name(const Memory& mem, DWORD dwAddress, PVOID pBuffer)
    {
        DWORD dwNameAddr = mem.read<DWORD>(dwAddress + m_pNetTableName);
        return mem.read(dwNameAddr, pBuffer, 128);
    }

    DWORD get_table(const Memory& mem, DWORD dwAddress)
    {
        return mem.read<DWORD>(dwAddress + m_pRecvTable);
    }

    DWORD get_next_class(const Memory& mem, DWORD dwAddress)
    {
        return mem.read<DWORD>(dwAddress + m_pNext);
    }

    DWORD scan_table(const Memory& mem, DWORD dwTableAddr, LPCSTR lpVarName, DWORD dwLevel)
    {
        for (int i = 0; i < get_prop_count(mem, dwTableAddr); i++)
        {
            DWORD dwPropAddr = get_prop_by_id(mem, dwTableAddr, i);
            if (!dwPropAddr)
                continue;

            char szPropName[128] = {0};
            if (!get_prop_name(mem, dwPropAddr, szPropName) || isdigit(szPropName[0]))
                continue;

            int iOffset = get_offset(mem, dwPropAddr);

            if (_stricmp(szPropName, lpVarName) == 0)
                return dwLevel + iOffset;

            DWORD dwTableAddr = get_data_table(mem, dwPropAddr);
            if (!dwTableAddr)
                continue;

            DWORD dwResult = scan_table(mem, dwTableAddr, lpVarName, dwLevel + iOffset);
            if (dwResult)
                return dwResult;
        }

        return 0;
    }

}

DWORD utl::find_netvar(const Memory& mem, DWORD dwStart, LPCSTR lpClassName, LPCSTR lpVarName)
{
    for (DWORD dwClass = dwStart; dwClass; dwClass = get_next_class(mem, dwClass))
    {
        DWORD dwTableAddr = get_table(mem, dwClass);

        char szTableName[128] = {0};
        if (!get_table_name(mem, dwTableAddr, szTableName))
            continue;

        if (_stricmp(szTableName, lpClassName) == 0)
            return scan_table(mem, dwTableAddr, lpVarName, 0);
    }

    return 0;
}