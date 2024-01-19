#pragma once
#include "utils/memory.hpp"

class Offsets {
public:
    static std::optional<Offsets> init(const Memory &mem);

public:
    DWORD dwClientState                = 0;
    DWORD dwClientState_ViewAngles     = 0;
    DWORD dwClientState_GetLocalPlayer = 0;
    DWORD dwLocalPlayer                = 0;
    DWORD dwEntityList                 = 0;
    DWORD dwEntityList2                = 0;
    DWORD dwForceJump                  = 0;
    DWORD dwForceAttack                = 0;
    DWORD dwGlowObjManager             = 0;
    DWORD modelAmbientMin              = 0;
    DWORD bDormant                     = 0;
    DWORD dwViewMatrix                 = 0;
    // netvars
    DWORD m_fFlags                     = 0;
    DWORD m_iCrosshairId               = 0;
    DWORD m_iTeamNum                   = 0;
    DWORD m_iHealth                    = 0;
    DWORD m_iGlowIndex                 = 0;
    DWORD m_hViewModel                 = 0;
    DWORD m_iViewModelIndex            = 0;
    DWORD m_flFallbackWear             = 0;
    DWORD m_nFallbackPaintKit          = 0;
    DWORD m_iItemIDHigh                = 0;
    DWORD m_iEntityQuality             = 0;
    DWORD m_iItemDefinitionIndex       = 0;
    DWORD m_hActiveWeapon              = 0;
    DWORD m_hMyWeapons                 = 0;
    DWORD m_nModelIndex                = 0;
    DWORD m_dwModelPrecache            = 0;
    DWORD m_aimPunchAngle              = 0;
    DWORD m_iShotsFired                = 0;
    DWORD m_dwBoneMatrix               = 0;
    DWORD m_vecOrigin                  = 0;
    DWORD m_vecViewOffset              = 0;
    DWORD m_bGunGameImmunity           = 0;
    DWORD m_vecVelocity                = 0;
    DWORD m_iFOV                       = 0;
    DWORD m_bIsScoped                  = 0;

private:
    bool initialize(const Memory& mem);
    bool scan_netvars(const Memory& mem, DWORD dwGetAllClasses);

};
