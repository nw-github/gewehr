#include <stdafx.h>

#include "utils/offsets.hpp"

#include "utils/utils.hpp"
#include "utils/netvars.hpp"

#define EXP(x) (const BYTE *)xorstr(x), sizeof(x) - 1

bool Offsets::initialize()
{
    utl::write_line(xorstr("\n[!] Scanning signatures..."));

    DWORD engine_base = g::memory->engine_dll.get_image_base();

    std::vector<BYTE> engine(g::memory->engine_dll.get_image_size(), '\0');
    if (!g::memory->read(engine_base, engine.data(), engine.size()))
        return false;

    // F3 0F 11 80 ? ? ? ? F3 0F 10 44 24 38
    dwClientState = utl::find_pattern(engine, engine_base,
        EXP("\xA1\xAA\xAA\xAA\xAA\x33\xD2\x6A\x00\x6A\x00\x33\xC9\x89\xB0"),
        0xAA, 0x1, 0x0, true, false);
    dwClientState_ViewAngles = utl::find_pattern(engine, engine_base,
        EXP("\xF3\x0F\x11\x86\xAA\xAA\xAA\xAA\xF3\x0F\x10\x44\x24\xAA\xF3\x0F\x11\x86"),
        0xAA, 0x4, 0x0, true, false);
    dwClientState_GetLocalPlayer = utl::find_pattern(engine, engine_base,
        EXP("\x8B\x80\xAA\xAA\xAA\xAA\x40\xC3"),
        0xAA, 0x2, 0x0, true, false);
    m_dwModelPrecache = utl::find_pattern(engine, engine_base,
        EXP("\x0C\x3B\x81\xAA\xAA\xAA\xAA\x75\x11\x8B\x45\x10\x83\xF8\x01\x7C\x09\x50\x83"),
        0xAA, 0x3, 0x0, true, false);
    modelAmbientMin = utl::find_pattern(engine, engine_base,
        EXP("\xF3\x0F\x10\x0D\xAA\xAA\xAA\xAA\xF3\x0F\x11\x4C\x24\xAA\x8B\x44\x24\x20\x35\xAA\xAA\xAA\xAA\x89\x44\x24\x0C"),
        0xAA, 0x4, 0x0, true, true);
    
    //-------------------------------------------------------------------------------------------

    DWORD client_base = g::memory->client_dll.get_image_base();

    std::vector<BYTE> client(g::memory->client_dll.get_image_size(), '\0');
    if (!g::memory->read(client_base, client.data(), client.size()))
        return false;

    dwEntityList = utl::find_pattern(client, client_base,
        EXP("\xBB\xAA\xAA\xAA\xAA\x83\xFF\x01\x0F\x8C\xAA\xAA\xAA\xAA\x3B\xF8"),
        0xAA, 1, 0, true, true);
    dwEntityList2 = utl::find_pattern(client, client_base,
        EXP("\xBB\xAA\xAA\xAA\xAA\x83\xFF\x01\x0F\x8C\xAA\xAA\xAA\xAA\x3B\xF8"),
        0xAA, 1, 0, true, false);
    dwLocalPlayer = utl::find_pattern(client, client_base,
        EXP("\x8D\x34\x85\xAA\xAA\xAA\xAA\x89\x15\xAA\xAA\xAA\xAA\x8B\x41\x08\x8B\x48\x04\x83\xF9\xFF"),
        0xAA, 3, 4, true, false);
    dwForceJump = utl::find_pattern(client, client_base,
        EXP("\x8B\x0D\xAA\xAA\xAA\xAA\x8B\xD6\x8B\xC1\x83\xCA\x02"),
        0xAA, 2, 0, true, true);
    dwForceAttack = utl::find_pattern(client, client_base,
        EXP("\x89\x0D\xAA\xAA\xAA\xAA\x8B\x0D\xAA\xAA\xAA\xAA\x8B\xF2\x8B\xC1\x83\xCE\x04"),
        0xAA, 2, 0, true, true);
    dwGlowObjManager = utl::find_pattern(client, client_base,
        EXP("\xA1\xAA\xAA\xAA\xAA\xA8\x01\x75\x4B"),
        0xAA, 1, 4, true, true);
    bDormant = utl::find_pattern(client, client_base,
        EXP("\x8A\x81\xAA\xAA\xAA\xAA\xC3\x32\xC0"),
        0xAA, 2, 8, true, false);
    dwViewMatrix = utl::find_pattern(client, client_base,
        EXP("\x0F\x10\x05\xAA\xAA\xAA\xAA\x8D\x85\xAA\xAA\xAA\xAA\xB9"),
        0xAA, 3, 176, true, true);
    
    //-------------------------------------------------------------------------------------------
    
    utl::write_line(xorstr("[+] dwClientState: {:#x}")        , dwClientState);
    utl::write_line(xorstr("[+] dwClientState_ViewAngles: {:#x}"), dwClientState_ViewAngles);
    utl::write_line(xorstr("[+] dwClientState_GetLocalPlayer: {:#x}"), dwClientState_GetLocalPlayer);
    utl::write_line(xorstr("[+] m_dwModelPrecache: {:#x}")    , m_dwModelPrecache);
    utl::write_line(xorstr("[+] modelAmbientMin: {:#x}")    , modelAmbientMin);

    utl::write_line(xorstr("[+] dwEntityList: {:#x}")        , dwEntityList);
    utl::write_line(xorstr("[+] dwEntityList2: {:#x}")        , dwEntityList2);
    utl::write_line(xorstr("[+] dwLocalPlayer: {:#x}")        , dwLocalPlayer);
    utl::write_line(xorstr("[+] dwForceJump: {:#x}")        , dwForceJump);
    utl::write_line(xorstr("[+] dwForceAttack: {:#x}")        , dwForceAttack);
    utl::write_line(xorstr("[+] dwGlowObjManager: {:#x}")    , dwGlowObjManager);
    utl::write_line(xorstr("[+] bDormant: {:#x}")            , bDormant);
    utl::write_line(xorstr("[+] dwViewMatrix: {:#x}")        , dwViewMatrix);

    //-------------------------------------------------------------------------------------------

    DWORD dwGetAllClasses = utl::find_pattern(client, client_base,
        EXP("\x44\x54\x5F\x54\x45\x57\x6F\x72\x6C\x64\x44\x65\x63\x61\x6C"),
        0xAA, 0, 0, false, false);
    dwGetAllClasses = utl::find_pattern(client, client_base,
        (PBYTE)&dwGetAllClasses, sizeof(PBYTE), 0x0, 0x2B, 0x0, true, false);
    
    return scan_netvars(dwGetAllClasses);
}

bool Offsets::scan_netvars(DWORD dwGetAllClasses) {
    utl::write_line(xorstr("\n[!] Scanning netvars..."));

    m_fFlags               = g::netvars->find_netvar(dwGetAllClasses, xorstr("DT_CSPlayer")            , xorstr("m_fFlags"));
    m_iCrosshairId         = g::netvars->find_netvar(dwGetAllClasses, xorstr("DT_CSPlayer")            , xorstr("m_bHasDefuser")) + 92;
    m_iGlowIndex           = g::netvars->find_netvar(dwGetAllClasses, xorstr("DT_CSPlayer")            , xorstr("m_flFlashDuration")) + 24;
    m_iShotsFired          = g::netvars->find_netvar(dwGetAllClasses, xorstr("DT_CSPlayer")            , xorstr("m_iShotsFired"));
    m_vecViewOffset        = g::netvars->find_netvar(dwGetAllClasses, xorstr("DT_CSPlayer")            , xorstr("m_vecViewOffset[0]"));
    m_bGunGameImmunity     = g::netvars->find_netvar(dwGetAllClasses, xorstr("DT_CSPlayer")            , xorstr("m_bGunGameImmunity"));
    m_vecVelocity          = g::netvars->find_netvar(dwGetAllClasses, xorstr("DT_CSPlayer")            , xorstr("m_vecVelocity[0]"));
    m_iFOV                 = g::netvars->find_netvar(dwGetAllClasses, xorstr("DT_CSPlayer")            , xorstr("m_iFOV"));
    m_bIsScoped            = g::netvars->find_netvar(dwGetAllClasses, xorstr("DT_CSPlayer")            , xorstr("m_bIsScoped"));
    m_iTeamNum             = g::netvars->find_netvar(dwGetAllClasses, xorstr("DT_BasePlayer")          , xorstr("m_iTeamNum"));
    m_iHealth              = g::netvars->find_netvar(dwGetAllClasses, xorstr("DT_BasePlayer")          , xorstr("m_iHealth"));
    m_hViewModel           = g::netvars->find_netvar(dwGetAllClasses, xorstr("DT_BasePlayer")          , xorstr("m_hViewModel[0]"));
    m_aimPunchAngle        = g::netvars->find_netvar(dwGetAllClasses, xorstr("DT_BasePlayer")          , xorstr("m_aimPunchAngle"));
    m_vecOrigin            = g::netvars->find_netvar(dwGetAllClasses, xorstr("DT_BasePlayer")          , xorstr("m_vecOrigin"));
    m_iViewModelIndex      = g::netvars->find_netvar(dwGetAllClasses, xorstr("DT_BaseCombatWeapon")    , xorstr("m_iViewModelIndex"));
    m_flFallbackWear       = g::netvars->find_netvar(dwGetAllClasses, xorstr("DT_BaseAttributableItem"), xorstr("m_flFallbackWear"));
    m_nFallbackPaintKit    = g::netvars->find_netvar(dwGetAllClasses, xorstr("DT_BaseAttributableItem"), xorstr("m_nFallbackPaintKit"));
    m_iItemIDHigh          = g::netvars->find_netvar(dwGetAllClasses, xorstr("DT_BaseAttributableItem"), xorstr("m_iItemIDHigh"));
    m_iEntityQuality       = g::netvars->find_netvar(dwGetAllClasses, xorstr("DT_BaseAttributableItem"), xorstr("m_iEntityQuality"));
    m_iItemDefinitionIndex = g::netvars->find_netvar(dwGetAllClasses, xorstr("DT_BaseAttributableItem"), xorstr("m_iItemDefinitionIndex"));
    m_hActiveWeapon        = g::netvars->find_netvar(dwGetAllClasses, xorstr("DT_BaseCombatCharacter") , xorstr("m_hActiveWeapon"));
    m_hMyWeapons           = g::netvars->find_netvar(dwGetAllClasses, xorstr("DT_BaseCombatCharacter") , xorstr("m_hMyWeapons"));
    m_nModelIndex          = g::netvars->find_netvar(dwGetAllClasses, xorstr("DT_BaseViewModel")       , xorstr("m_nModelIndex"));
    m_dwBoneMatrix         = g::netvars->find_netvar(dwGetAllClasses, xorstr("DT_BaseAnimating")       , xorstr("m_nForceBone")) + 28;

    utl::write_line(xorstr("[+] m_fFlags: {:#x}")              , m_fFlags);
    utl::write_line(xorstr("[+] m_iCrosshairId: {:#x}")        , m_iCrosshairId);
    utl::write_line(xorstr("[+] m_iTeamNum: {:#x}")            , m_iTeamNum);
    utl::write_line(xorstr("[+] m_iHealth: {:#x}")             , m_iHealth);
    utl::write_line(xorstr("[+] m_iGlowIndex: {:#x}")          , m_iGlowIndex);
    utl::write_line(xorstr("[+] m_hViewModel: {:#x}")          , m_hViewModel);
    utl::write_line(xorstr("[+] m_iViewModelIndex: {:#x}")     , m_iViewModelIndex);
    utl::write_line(xorstr("[+] m_flFallbackWear: {:#x}")      , m_flFallbackWear);
    utl::write_line(xorstr("[+] m_nFallbackPaintKit: {:#x}")   , m_nFallbackPaintKit);
    utl::write_line(xorstr("[+] m_iItemIDHigh: {:#x}")         , m_iItemIDHigh);
    utl::write_line(xorstr("[+] m_iEntityQuality: {:#x}")      , m_iEntityQuality);
    utl::write_line(xorstr("[+] m_iItemDefinitionIndex: {:#x}"), m_iItemDefinitionIndex);
    utl::write_line(xorstr("[+] m_hActiveWeapon: {:#x}")       , m_hActiveWeapon);
    utl::write_line(xorstr("[+] m_hMyWeapons: {:#x}")          , m_hMyWeapons);
    utl::write_line(xorstr("[+] m_nModelIndex: {:#x}")         , m_nModelIndex);
    utl::write_line(xorstr("[+] m_aimPunchAngle: {:#x}")       , m_aimPunchAngle);
    utl::write_line(xorstr("[+] m_iShotsFired: {:#x}")         , m_iShotsFired);
    utl::write_line(xorstr("[+] m_dwBoneMatrix: {:#x}")        , m_dwBoneMatrix);
    utl::write_line(xorstr("[+] m_vecOrigin: {:#x}")           , m_vecOrigin);
    utl::write_line(xorstr("[+] m_bGunGameImmunity: {:#x}")    , m_bGunGameImmunity);
    utl::write_line(xorstr("[+] m_iFOV: {:#x}")                , m_iFOV);
    utl::write_line(xorstr("[+] m_bIsScoped: {:#x}")           , m_bIsScoped);
    utl::write_line(xorstr("[+] m_vecVelocity: {:#x}")         , m_vecVelocity);
    utl::write_line(xorstr("[+] m_vecViewOffset: {:#x}")       , m_vecViewOffset);

    return true;
}