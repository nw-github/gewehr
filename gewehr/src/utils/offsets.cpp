#include <stdafx.h>

#include "utils/offsets.hpp"
#include "utils/utils.hpp"

namespace {
    bool check_pattern(const BYTE *bytes, std::span<const BYTE> pattern, BYTE wildcard) {
        for (size_t i = 0; i < pattern.size(); i++) {
            if (pattern[i] != wildcard && bytes[i] != pattern[i]) {
                return false;
            }
        }
        return true;
    }

    DWORD find_pattern(
        std::span<const BYTE> buffer,
        DWORD base,
        std::span<const BYTE> pattern,
        BYTE wildcard,
        UINT offset,
        UINT extra,
        bool relative,
        bool subtract
    ) {
        auto bytes = buffer.data();
        for (DWORD i = 0; i < buffer.size() - pattern.size(); i++)  {
            if (check_pattern(bytes + i, pattern, wildcard)) {
                DWORD addr = base + i + offset;
                if (relative) {
                    addr = *(DWORD *)(bytes + i + offset);
                }
                if (subtract) {
                    addr -= base;
                }
                return addr + extra;
            }
        }
        return 0;
    }
}

namespace netvars {
    constexpr const DWORD nCRecvPropSize = 0x3C;

    // CRecvProp struct offsets
    constexpr const DWORD m_pVarName = 0x0;
    constexpr const DWORD m_pDataTable = 0x28;
    constexpr const DWORD m_iOffset = 0x2C;

    // CRecvTable struct offsets
    constexpr const DWORD m_pProps = 0x0;
    constexpr const DWORD m_nProps = 0x4;
    constexpr const DWORD m_pNetTableName = 0xC;

    // CClientClass struct offsets
    constexpr const DWORD m_pRecvTable = 0xC;
    constexpr const DWORD m_pNext = 0x10;

    DWORD scan_table(const Memory &mem, DWORD table_addr, std::string_view var_name, DWORD level) {
        auto props_count = mem.read<int>(table_addr + m_nProps);
        for (int i = 0; i < props_count; i++) {
            auto dwPropAddr = mem.read<DWORD>(table_addr + m_pProps);
            if (!dwPropAddr) {
                continue;
            }
            dwPropAddr += nCRecvPropSize * i;

            auto name_addr = mem.read<DWORD>(dwPropAddr + m_pVarName);
            auto prop_name = mem.read<std::array<char, 128>>(name_addr);
            if (isdigit(prop_name[0])) {
                continue;
            }

            auto offset = mem.read<int>(dwPropAddr + m_iOffset);
            if (utl::icompare(prop_name.data(), var_name)) {
                return level + offset;
            }

            auto table_addr = mem.read<DWORD>(dwPropAddr + m_pDataTable);
            if (!table_addr) {
                continue;
            }

            if (auto result = scan_table(mem, table_addr, var_name, level + offset)) {
                return result;
            }
        }

        return 0;
    }

    DWORD find(const Memory& mem, DWORD start, std::string_view class_name, std::string_view var_name) {
        for (DWORD clazz = start; clazz; clazz = mem.read<DWORD>(clazz + m_pNext)) {
            auto table_addr = mem.read<DWORD>(clazz + m_pRecvTable);
            auto name_addr = mem.read<DWORD>(table_addr + m_pNetTableName);
            auto table_name = mem.read<std::array<char, 128>>(name_addr);
            if (utl::icompare(table_name.data(), class_name)) {
                return scan_table(mem, table_addr, var_name, 0);
            }
        }
        return 0;
    }
}

std::optional<Offsets> Offsets::init(const Memory &mem) {
    Offsets offsets;
    if (!offsets.initialize(mem)) {
        return std::nullopt;
    }

    return offsets;
}

bool Offsets::initialize(const Memory &mem) {
    utl::println(xorstr("\n[!] Scanning signatures..."));

    DWORD engine_base = mem.engine_dll.base_addr;

    std::vector<BYTE> engine(mem.engine_dll.size, '\0');
    if (!mem.read(engine_base, engine.data(), engine.size())) {
        return false;
    }

    DWORD client_base = mem.client_dll.base_addr;

    std::vector<BYTE> client(mem.client_dll.size, '\0');
    if (!mem.read(client_base, client.data(), client.size())) {
        return false;
    }

#define EXP(x) std::span((const BYTE *)xorstr(x), sizeof(x) - 1)
#define FIND_PATTERN(var, data, base, patt, wildcard, offs, extra, rel, sub) \
    if (!(var = find_pattern(data, base, EXP(patt), wildcard, offs, extra, rel, sub))) {\
        utl::println(xorstr("[!] Offset "#var" could not be found!"));\
        return false;\
    }\
    utl::println(xorstr("[+] "#var": {:#x}"), var);

    // F3 0F 11 80 ? ? ? ? F3 0F 10 44 24 38
    FIND_PATTERN(dwClientState, engine, engine_base,
        "\xA1\xAA\xAA\xAA\xAA\x33\xD2\x6A\x00\x6A\x00\x33\xC9\x89\xB0",
        0xAA, 0x1, 0x0, true, false);
    FIND_PATTERN(dwClientState_ViewAngles, engine, engine_base,
        "\xF3\x0F\x11\x86\xAA\xAA\xAA\xAA\xF3\x0F\x10\x44\x24\xAA\xF3\x0F\x11\x86",
        0xAA, 0x4, 0x0, true, false);
    FIND_PATTERN(dwClientState_GetLocalPlayer, engine, engine_base,
        "\x8B\x80\xAA\xAA\xAA\xAA\x40\xC3",
        0xAA, 0x2, 0x0, true, false);
    FIND_PATTERN(m_dwModelPrecache, engine, engine_base,
        "\x0C\x3B\x81\xAA\xAA\xAA\xAA\x75\x11\x8B\x45\x10\x83\xF8\x01\x7C\x09\x50\x83",
        0xAA, 0x3, 0x0, true, false);
    FIND_PATTERN(modelAmbientMin, engine, engine_base,
        "\xF3\x0F\x10\x0D\xAA\xAA\xAA\xAA\xF3\x0F\x11\x4C\x24\xAA\x8B\x44\x24\x20\x35\xAA\xAA\xAA\xAA\x89\x44\x24\x0C",
        0xAA, 0x4, 0x0, true, true);

    //-------------------------------------------------------------------------------------------

    FIND_PATTERN(dwEntityList, client, client_base,
        "\xBB\xAA\xAA\xAA\xAA\x83\xFF\x01\x0F\x8C\xAA\xAA\xAA\xAA\x3B\xF8",
        0xAA, 1, 0, true, true);
    FIND_PATTERN(dwEntityList2, client, client_base,
        "\xBB\xAA\xAA\xAA\xAA\x83\xFF\x01\x0F\x8C\xAA\xAA\xAA\xAA\x3B\xF8",
        0xAA, 1, 0, true, false);
    FIND_PATTERN(dwLocalPlayer, client, client_base,
        "\x8D\x34\x85\xAA\xAA\xAA\xAA\x89\x15\xAA\xAA\xAA\xAA\x8B\x41\x08\x8B\x48\x04\x83\xF9\xFF",
        0xAA, 3, 4, true, false);
    FIND_PATTERN(dwForceJump, client, client_base,
        "\x8B\x0D\xAA\xAA\xAA\xAA\x8B\xD6\x8B\xC1\x83\xCA\x02",
        0xAA, 2, 0, true, true);
    FIND_PATTERN(dwForceAttack, client, client_base,
        "\x89\x0D\xAA\xAA\xAA\xAA\x8B\x0D\xAA\xAA\xAA\xAA\x8B\xF2\x8B\xC1\x83\xCE\x04",
        0xAA, 2, 0, true, true);
    FIND_PATTERN(dwGlowObjManager, client, client_base,
        "\xA1\xAA\xAA\xAA\xAA\xA8\x01\x75\x4B",
        0xAA, 1, 4, true, true);
    FIND_PATTERN(bDormant, client, client_base,
        "\x8A\x81\xAA\xAA\xAA\xAA\xC3\x32\xC0",
        0xAA, 2, 8, true, false);
    FIND_PATTERN(dwViewMatrix, client, client_base,
        "\x0F\x10\x05\xAA\xAA\xAA\xAA\x8D\x85\xAA\xAA\xAA\xAA\xB9",
        0xAA, 3, 176, true, true);

    //-------------------------------------------------------------------------------------------

    DWORD dwGetAllClasses = find_pattern(client, client_base,
        EXP("\x44\x54\x5F\x54\x45\x57\x6F\x72\x6C\x64\x44\x65\x63\x61\x6C"),
        0xAA, 0, 0, false, false);
    dwGetAllClasses = find_pattern(client, client_base,
        {(PBYTE)&dwGetAllClasses, sizeof(PBYTE)}, 0x0, 0x2B, 0x0, true, false);

#undef EXP
#undef FIND_PATTERN

    return scan_netvars(mem, dwGetAllClasses);
}

bool Offsets::scan_netvars(const Memory &mem, DWORD dwGetAllClasses) {
    utl::println(xorstr("\n[!] Scanning netvars..."));

    #define FIND_NETVAR(var, class_name, var_name, offset) \
        if (!(var = netvars::find(mem, dwGetAllClasses, xorstr(class_name), xorstr(var_name)))) {\
            utl::println(xorstr("[!] Netvar "#var" could not be found!"));\
            return false;\
        }\
        var += offset;\
        utl::println(xorstr("[+] "#var": {:#x}"), var);

    FIND_NETVAR(m_fFlags, "DT_CSPlayer", "m_fFlags", 0);
    FIND_NETVAR(m_iCrosshairId, "DT_CSPlayer", "m_bHasDefuser", 92);
    FIND_NETVAR(m_iGlowIndex, "DT_CSPlayer", "m_flFlashDuration", 24);
    FIND_NETVAR(m_iShotsFired, "DT_CSPlayer", "m_iShotsFired", 0);
    FIND_NETVAR(m_vecViewOffset, "DT_CSPlayer", "m_vecViewOffset[0]", 0);
    FIND_NETVAR(m_bGunGameImmunity, "DT_CSPlayer", "m_bGunGameImmunity", 0);
    FIND_NETVAR(m_vecVelocity, "DT_CSPlayer", "m_vecVelocity[0]", 0);
    FIND_NETVAR(m_iFOV, "DT_CSPlayer", "m_iFOV", 0);
    FIND_NETVAR(m_bIsScoped, "DT_CSPlayer", "m_bIsScoped", 0);
    FIND_NETVAR(m_iTeamNum, "DT_BasePlayer", "m_iTeamNum", 0);
    FIND_NETVAR(m_iHealth, "DT_BasePlayer", "m_iHealth", 0);
    FIND_NETVAR(m_hViewModel, "DT_BasePlayer", "m_hViewModel[0]", 0);
    FIND_NETVAR(m_aimPunchAngle, "DT_BasePlayer", "m_aimPunchAngle", 0);
    FIND_NETVAR(m_vecOrigin, "DT_BasePlayer", "m_vecOrigin", 0);
    FIND_NETVAR(m_iViewModelIndex, "DT_BaseCombatWeapon", "m_iViewModelIndex", 0);
    FIND_NETVAR(m_flFallbackWear, "DT_BaseAttributableItem", "m_flFallbackWear", 0);
    FIND_NETVAR(m_nFallbackPaintKit, "DT_BaseAttributableItem", "m_nFallbackPaintKit", 0);
    FIND_NETVAR(m_iItemIDHigh, "DT_BaseAttributableItem", "m_iItemIDHigh", 0);
    FIND_NETVAR(m_iEntityQuality, "DT_BaseAttributableItem", "m_iEntityQuality", 0);
    FIND_NETVAR(m_iItemDefinitionIndex, "DT_BaseAttributableItem", "m_iItemDefinitionIndex", 0);
    FIND_NETVAR(m_hActiveWeapon, "DT_BaseCombatCharacter", "m_hActiveWeapon", 0);
    FIND_NETVAR(m_hMyWeapons, "DT_BaseCombatCharacter", "m_hMyWeapons", 0);
    FIND_NETVAR(m_nModelIndex, "DT_BaseViewModel", "m_nModelIndex", 0);
    FIND_NETVAR(m_dwBoneMatrix, "DT_BaseAnimating", "m_nForceBone", 28);

    #undef FIND_NETVAR

    return true;
}