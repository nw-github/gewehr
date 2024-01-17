#include <stdafx.h>

#include "utils/offsets.hpp"
#include "utils/utils.hpp"

#define EXP(x) (const BYTE *)xorstr(x), sizeof(x) - 1

namespace {
    bool check_pattern(const BYTE *bytes, const BYTE *pattern, UINT length, BYTE wildcard) {
        for (UINT i = 0; i < length; i++)
            if (pattern[i] != wildcard && bytes[i] != pattern[i])
                return false;
        return true;
    }

    DWORD find_pattern(
        const std::vector<BYTE> &buffer, 
        DWORD module_base, 
        const BYTE *pattern, 
        UINT length,
        BYTE wildcard, 
        UINT offset, 
        UINT extra, 
        bool relative, 
        bool subtract
    ) {
        auto bytes = buffer.data();
        DWORD size = buffer.size();

        DWORD dwAddress = 0;
        for (DWORD i = 0; i < size - length; i++)
        {
            if (check_pattern(bytes + i, pattern, length, wildcard))
            {
                dwAddress = module_base + i + offset;
                if (relative)
                    dwAddress = *(DWORD *)(bytes + i + offset);
                if (subtract)
                    dwAddress -= module_base;

                dwAddress += extra;
                break;
            }
        }
        return dwAddress;
    }
}

std::optional<Offsets> Offsets::init(const Memory& mem) {
    Offsets offsets;
    if (!offsets.initialize(mem)) {
        return std::nullopt;
    }

    return offsets;
}

bool Offsets::initialize(const Memory& mem)
{
    utl::println(xorstr("\n[!] Scanning signatures..."));

    DWORD engine_base = mem.engine_dll.get_image_base();

    std::vector<BYTE> engine(mem.engine_dll.get_image_size(), '\0');
    if (!mem.read(engine_base, engine.data(), engine.size()))
        return false;

    // F3 0F 11 80 ? ? ? ? F3 0F 10 44 24 38
    dwClientState = find_pattern(engine, engine_base,
        EXP("\xA1\xAA\xAA\xAA\xAA\x33\xD2\x6A\x00\x6A\x00\x33\xC9\x89\xB0"),
        0xAA, 0x1, 0x0, true, false);
    dwClientState_ViewAngles = find_pattern(engine, engine_base,
        EXP("\xF3\x0F\x11\x86\xAA\xAA\xAA\xAA\xF3\x0F\x10\x44\x24\xAA\xF3\x0F\x11\x86"),
        0xAA, 0x4, 0x0, true, false);
    dwClientState_GetLocalPlayer = find_pattern(engine, engine_base,
        EXP("\x8B\x80\xAA\xAA\xAA\xAA\x40\xC3"),
        0xAA, 0x2, 0x0, true, false);
    m_dwModelPrecache = find_pattern(engine, engine_base,
        EXP("\x0C\x3B\x81\xAA\xAA\xAA\xAA\x75\x11\x8B\x45\x10\x83\xF8\x01\x7C\x09\x50\x83"),
        0xAA, 0x3, 0x0, true, false);
    modelAmbientMin = find_pattern(engine, engine_base,
        EXP("\xF3\x0F\x10\x0D\xAA\xAA\xAA\xAA\xF3\x0F\x11\x4C\x24\xAA\x8B\x44\x24\x20\x35\xAA\xAA\xAA\xAA\x89\x44\x24\x0C"),
        0xAA, 0x4, 0x0, true, true);
    
    //-------------------------------------------------------------------------------------------

    DWORD client_base = mem.client_dll.get_image_base();

    std::vector<BYTE> client(mem.client_dll.get_image_size(), '\0');
    if (!mem.read(client_base, client.data(), client.size()))
        return false;

    dwEntityList = find_pattern(client, client_base,
        EXP("\xBB\xAA\xAA\xAA\xAA\x83\xFF\x01\x0F\x8C\xAA\xAA\xAA\xAA\x3B\xF8"),
        0xAA, 1, 0, true, true);
    dwEntityList2 = find_pattern(client, client_base,
        EXP("\xBB\xAA\xAA\xAA\xAA\x83\xFF\x01\x0F\x8C\xAA\xAA\xAA\xAA\x3B\xF8"),
        0xAA, 1, 0, true, false);
    dwLocalPlayer = find_pattern(client, client_base,
        EXP("\x8D\x34\x85\xAA\xAA\xAA\xAA\x89\x15\xAA\xAA\xAA\xAA\x8B\x41\x08\x8B\x48\x04\x83\xF9\xFF"),
        0xAA, 3, 4, true, false);
    dwForceJump = find_pattern(client, client_base,
        EXP("\x8B\x0D\xAA\xAA\xAA\xAA\x8B\xD6\x8B\xC1\x83\xCA\x02"),
        0xAA, 2, 0, true, true);
    dwForceAttack = find_pattern(client, client_base,
        EXP("\x89\x0D\xAA\xAA\xAA\xAA\x8B\x0D\xAA\xAA\xAA\xAA\x8B\xF2\x8B\xC1\x83\xCE\x04"),
        0xAA, 2, 0, true, true);
    dwGlowObjManager = find_pattern(client, client_base,
        EXP("\xA1\xAA\xAA\xAA\xAA\xA8\x01\x75\x4B"),
        0xAA, 1, 4, true, true);
    bDormant = find_pattern(client, client_base,
        EXP("\x8A\x81\xAA\xAA\xAA\xAA\xC3\x32\xC0"),
        0xAA, 2, 8, true, false);
    dwViewMatrix = find_pattern(client, client_base,
        EXP("\x0F\x10\x05\xAA\xAA\xAA\xAA\x8D\x85\xAA\xAA\xAA\xAA\xB9"),
        0xAA, 3, 176, true, true);
    
    //-------------------------------------------------------------------------------------------
    
    utl::println(xorstr("[+] dwClientState: {:#x}")        , dwClientState);
    utl::println(xorstr("[+] dwClientState_ViewAngles: {:#x}"), dwClientState_ViewAngles);
    utl::println(xorstr("[+] dwClientState_GetLocalPlayer: {:#x}"), dwClientState_GetLocalPlayer);
    utl::println(xorstr("[+] m_dwModelPrecache: {:#x}")    , m_dwModelPrecache);
    utl::println(xorstr("[+] modelAmbientMin: {:#x}")    , modelAmbientMin);

    utl::println(xorstr("[+] dwEntityList: {:#x}")        , dwEntityList);
    utl::println(xorstr("[+] dwEntityList2: {:#x}")        , dwEntityList2);
    utl::println(xorstr("[+] dwLocalPlayer: {:#x}")        , dwLocalPlayer);
    utl::println(xorstr("[+] dwForceJump: {:#x}")        , dwForceJump);
    utl::println(xorstr("[+] dwForceAttack: {:#x}")        , dwForceAttack);
    utl::println(xorstr("[+] dwGlowObjManager: {:#x}")    , dwGlowObjManager);
    utl::println(xorstr("[+] bDormant: {:#x}")            , bDormant);
    utl::println(xorstr("[+] dwViewMatrix: {:#x}")        , dwViewMatrix);

    //-------------------------------------------------------------------------------------------

    DWORD dwGetAllClasses = find_pattern(client, client_base,
        EXP("\x44\x54\x5F\x54\x45\x57\x6F\x72\x6C\x64\x44\x65\x63\x61\x6C"),
        0xAA, 0, 0, false, false);
    dwGetAllClasses = find_pattern(client, client_base,
        (PBYTE)&dwGetAllClasses, sizeof(PBYTE), 0x0, 0x2B, 0x0, true, false);
    
    return scan_netvars(mem, dwGetAllClasses);
}

bool Offsets::scan_netvars(const Memory& mem, DWORD dwGetAllClasses) {
    utl::println(xorstr("\n[!] Scanning netvars..."));

    m_fFlags               = netvars::find(mem, dwGetAllClasses, xorstr("DT_CSPlayer")            , xorstr("m_fFlags"));
    m_iCrosshairId         = netvars::find(mem, dwGetAllClasses, xorstr("DT_CSPlayer")            , xorstr("m_bHasDefuser")) + 92;
    m_iGlowIndex           = netvars::find(mem, dwGetAllClasses, xorstr("DT_CSPlayer")            , xorstr("m_flFlashDuration")) + 24;
    m_iShotsFired          = netvars::find(mem, dwGetAllClasses, xorstr("DT_CSPlayer")            , xorstr("m_iShotsFired"));
    m_vecViewOffset        = netvars::find(mem, dwGetAllClasses, xorstr("DT_CSPlayer")            , xorstr("m_vecViewOffset[0]"));
    m_bGunGameImmunity     = netvars::find(mem, dwGetAllClasses, xorstr("DT_CSPlayer")            , xorstr("m_bGunGameImmunity"));
    m_vecVelocity          = netvars::find(mem, dwGetAllClasses, xorstr("DT_CSPlayer")            , xorstr("m_vecVelocity[0]"));
    m_iFOV                 = netvars::find(mem, dwGetAllClasses, xorstr("DT_CSPlayer")            , xorstr("m_iFOV"));
    m_bIsScoped            = netvars::find(mem, dwGetAllClasses, xorstr("DT_CSPlayer")            , xorstr("m_bIsScoped"));
    m_iTeamNum             = netvars::find(mem, dwGetAllClasses, xorstr("DT_BasePlayer")          , xorstr("m_iTeamNum"));
    m_iHealth              = netvars::find(mem, dwGetAllClasses, xorstr("DT_BasePlayer")          , xorstr("m_iHealth"));
    m_hViewModel           = netvars::find(mem, dwGetAllClasses, xorstr("DT_BasePlayer")          , xorstr("m_hViewModel[0]"));
    m_aimPunchAngle        = netvars::find(mem, dwGetAllClasses, xorstr("DT_BasePlayer")          , xorstr("m_aimPunchAngle"));
    m_vecOrigin            = netvars::find(mem, dwGetAllClasses, xorstr("DT_BasePlayer")          , xorstr("m_vecOrigin"));
    m_iViewModelIndex      = netvars::find(mem, dwGetAllClasses, xorstr("DT_BaseCombatWeapon")    , xorstr("m_iViewModelIndex"));
    m_flFallbackWear       = netvars::find(mem, dwGetAllClasses, xorstr("DT_BaseAttributableItem"), xorstr("m_flFallbackWear"));
    m_nFallbackPaintKit    = netvars::find(mem, dwGetAllClasses, xorstr("DT_BaseAttributableItem"), xorstr("m_nFallbackPaintKit"));
    m_iItemIDHigh          = netvars::find(mem, dwGetAllClasses, xorstr("DT_BaseAttributableItem"), xorstr("m_iItemIDHigh"));
    m_iEntityQuality       = netvars::find(mem, dwGetAllClasses, xorstr("DT_BaseAttributableItem"), xorstr("m_iEntityQuality"));
    m_iItemDefinitionIndex = netvars::find(mem, dwGetAllClasses, xorstr("DT_BaseAttributableItem"), xorstr("m_iItemDefinitionIndex"));
    m_hActiveWeapon        = netvars::find(mem, dwGetAllClasses, xorstr("DT_BaseCombatCharacter") , xorstr("m_hActiveWeapon"));
    m_hMyWeapons           = netvars::find(mem, dwGetAllClasses, xorstr("DT_BaseCombatCharacter") , xorstr("m_hMyWeapons"));
    m_nModelIndex          = netvars::find(mem, dwGetAllClasses, xorstr("DT_BaseViewModel")       , xorstr("m_nModelIndex"));
    m_dwBoneMatrix         = netvars::find(mem, dwGetAllClasses, xorstr("DT_BaseAnimating")       , xorstr("m_nForceBone")) + 28;

    utl::println(xorstr("[+] m_fFlags: {:#x}")              , m_fFlags);
    utl::println(xorstr("[+] m_iCrosshairId: {:#x}")        , m_iCrosshairId);
    utl::println(xorstr("[+] m_iTeamNum: {:#x}")            , m_iTeamNum);
    utl::println(xorstr("[+] m_iHealth: {:#x}")             , m_iHealth);
    utl::println(xorstr("[+] m_iGlowIndex: {:#x}")          , m_iGlowIndex);
    utl::println(xorstr("[+] m_hViewModel: {:#x}")          , m_hViewModel);
    utl::println(xorstr("[+] m_iViewModelIndex: {:#x}")     , m_iViewModelIndex);
    utl::println(xorstr("[+] m_flFallbackWear: {:#x}")      , m_flFallbackWear);
    utl::println(xorstr("[+] m_nFallbackPaintKit: {:#x}")   , m_nFallbackPaintKit);
    utl::println(xorstr("[+] m_iItemIDHigh: {:#x}")         , m_iItemIDHigh);
    utl::println(xorstr("[+] m_iEntityQuality: {:#x}")      , m_iEntityQuality);
    utl::println(xorstr("[+] m_iItemDefinitionIndex: {:#x}"), m_iItemDefinitionIndex);
    utl::println(xorstr("[+] m_hActiveWeapon: {:#x}")       , m_hActiveWeapon);
    utl::println(xorstr("[+] m_hMyWeapons: {:#x}")          , m_hMyWeapons);
    utl::println(xorstr("[+] m_nModelIndex: {:#x}")         , m_nModelIndex);
    utl::println(xorstr("[+] m_aimPunchAngle: {:#x}")       , m_aimPunchAngle);
    utl::println(xorstr("[+] m_iShotsFired: {:#x}")         , m_iShotsFired);
    utl::println(xorstr("[+] m_dwBoneMatrix: {:#x}")        , m_dwBoneMatrix);
    utl::println(xorstr("[+] m_vecOrigin: {:#x}")           , m_vecOrigin);
    utl::println(xorstr("[+] m_bGunGameImmunity: {:#x}")    , m_bGunGameImmunity);
    utl::println(xorstr("[+] m_iFOV: {:#x}")                , m_iFOV);
    utl::println(xorstr("[+] m_bIsScoped: {:#x}")           , m_bIsScoped);
    utl::println(xorstr("[+] m_vecVelocity: {:#x}")         , m_vecVelocity);
    utl::println(xorstr("[+] m_vecViewOffset: {:#x}")       , m_vecViewOffset);

    return true;
}