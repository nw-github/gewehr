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

    bool get_prop_name(DWORD dwAddress, PVOID pBuffer)
    {
        DWORD dwNameAddr;
        return g::memory->read(dwAddress + m_pVarName, &dwNameAddr, sizeof(DWORD)) &&
               g::memory->read(dwNameAddr, pBuffer, 128);
    }

    DWORD get_data_table(DWORD dwAddress)
    {
        return g::memory->read<DWORD>(dwAddress + m_pDataTable);
    }

    int get_offset(DWORD dwAddress)
    {
        return g::memory->read<int>(dwAddress + m_iOffset);
    }

    DWORD get_prop_by_id(DWORD dwAddress, int iIndex)
    {
        DWORD dwPropAddr = g::memory->read<DWORD>(dwAddress + m_pProps);
        return dwPropAddr + nCRecvPropSize * iIndex;
    }

    int get_prop_count(DWORD dwAddress)
    {
        return g::memory->read<int>(dwAddress + m_nProps);
    }

    bool get_table_name(DWORD dwAddress, PVOID pBuffer)
    {
        DWORD dwNameAddr;
        return g::memory->read(dwAddress + m_pNetTableName, &dwNameAddr, sizeof(DWORD)) &&
               g::memory->read(dwNameAddr, pBuffer, 128);
    }

    DWORD get_table(DWORD dwAddress)
    {
        return g::memory->read<DWORD>(dwAddress + m_pRecvTable);
    }

    DWORD get_next_class(DWORD dwAddress)
    {
        return g::memory->read<DWORD>(dwAddress + m_pNext);
    }

    DWORD scan_table(DWORD dwTableAddr, LPCSTR lpVarName, DWORD dwLevel)
    {
        for (int i = 0; i < get_prop_count(dwTableAddr); i++)
        {
            DWORD dwPropAddr = get_prop_by_id(dwTableAddr, i);
            if (!dwPropAddr)
                continue;

            char szPropName[128] = {0};
            if (!get_prop_name(dwPropAddr, szPropName) || isdigit(szPropName[0]))
                continue;

            int iOffset = get_offset(dwPropAddr);

            if (_stricmp(szPropName, lpVarName) == 0)
                return dwLevel + iOffset;

            DWORD dwTableAddr = get_data_table(dwPropAddr);
            if (!dwTableAddr)
                continue;

            DWORD dwResult = scan_table(dwTableAddr, lpVarName, dwLevel + iOffset);
            if (dwResult)
                return dwResult;
        }

        return 0;
    }

}

DWORD netvars::find(DWORD dwStart, LPCSTR lpClassName, LPCSTR lpVarName)
{
    for (DWORD dwClass = dwStart; dwClass; dwClass = get_next_class(dwClass))
    {
        DWORD dwTableAddr = get_table(dwClass);

        char szTableName[128] = {0};
        if (!get_table_name(dwTableAddr, szTableName))
            continue;

        if (_stricmp(szTableName, lpClassName) == 0)
            return scan_table(dwTableAddr, lpVarName, 0);
    }

    return 0;
}