#include <stdafx.h>

#include "utils/entity.hpp"

#include "utils/memory.hpp"
#include "utils/offsets.hpp"

BasePlayer::BasePlayer(const Memory &mem, const Offsets &offsets, int ent_index)
    : BasePlayer(mem, offsets)
{
    m_dwBaseAddr = mem.read<DWORD>(
        mem.client_dll.get_image_base() + offsets.dwEntityList + (ent_index * 0x10)
    );
}

int BasePlayer::m_iTeamNum()
{
    return mem.read<int>(m_dwBaseAddr + offsets.m_iTeamNum);
}

int BasePlayer::m_iHealth()
{
    return mem.read<int>(m_dwBaseAddr + offsets.m_iHealth);
}

int BasePlayer::m_iGlowIndex()
{
    return mem.read<int>(m_dwBaseAddr + offsets.m_iGlowIndex);
}

int BasePlayer::m_iCrosshairId()
{
    return mem.read<int>(m_dwBaseAddr + offsets.m_iCrosshairId);
}

int BasePlayer::m_iShotsFired()
{
    return mem.read<int>(m_dwBaseAddr + offsets.m_iShotsFired);
}

bool BasePlayer::m_bDormant()
{
    return mem.read<bool>(m_dwBaseAddr + offsets.bDormant);
}

bool BasePlayer::m_bGunGameImmunity()
{
    return mem.read<bool>(m_dwBaseAddr + offsets.m_bGunGameImmunity);
}

bool BasePlayer::m_bIsScoped()
{
    return mem.read<bool>(m_dwBaseAddr + offsets.m_bIsScoped);
}

BYTE BasePlayer::m_fFlags()
{
    return mem.read<BYTE>(m_dwBaseAddr + offsets.m_fFlags);
}

QAngle BasePlayer::m_aimPunchAngle()
{
    return mem.read<QAngle>(m_dwBaseAddr + offsets.m_aimPunchAngle);
}

Vector BasePlayer::m_vecOrigin()
{
    return mem.read<Vector>(m_dwBaseAddr + offsets.m_vecOrigin);
}

Vector BasePlayer::m_vecVelocity()
{
    return mem.read<Vector>(m_dwBaseAddr + offsets.m_vecVelocity);
}

Vector BasePlayer::m_vecViewOffset()
{
    return mem.read<Vector>(m_dwBaseAddr + offsets.m_vecViewOffset);
}

Vector BasePlayer::GetBonePos(int bone)
{
    Vector pos;
    DWORD boneMatrix = mem.read<DWORD>(m_dwBaseAddr + offsets.m_dwBoneMatrix);

    pos.x = mem.read<float>(boneMatrix + (0x30 * bone) + 0x0C);
    pos.y = mem.read<float>(boneMatrix + (0x30 * bone) + 0x1C);
    pos.z = mem.read<float>(boneMatrix + (0x30 * bone) + 0x2C);
    return pos;
}

Vector BasePlayer::GetEyePos()
{
    Vector eyePos = m_vecOrigin();
    eyePos.z += mem.read<Vector>(m_dwBaseAddr + offsets.m_vecViewOffset).z;
    return eyePos;
}

//------------------------------------------

LocalPlayer::LocalPlayer(const Memory &mem, const Offsets &offsets)
    : BasePlayer(mem, offsets)
{
    m_dwBaseAddr = mem.read<DWORD>(offsets.dwLocalPlayer);
    m_dwClientState = mem.read<DWORD>(offsets.dwClientState);
}

QAngle LocalPlayer::GetViewAngles()
{
    return mem.read<QAngle>(m_dwClientState + offsets.dwClientState_ViewAngles);
}

int LocalPlayer::EntIndex()
{
    return mem.read<int>(m_dwClientState + offsets.dwClientState_GetLocalPlayer) + 1;
}

int LocalPlayer::m_iFOV()
{
    return mem.read<int>(m_dwBaseAddr + offsets.m_iFOV);
}

void LocalPlayer::SetViewAngles(const QAngle &angles)
{
    mem.write(m_dwClientState + offsets.dwClientState_ViewAngles, angles);
}

void LocalPlayer::force_jump(DWORD jump)
{
    mem.write<DWORD>(mem.client_dll.get_image_base() + offsets.dwForceJump, jump);
}

void LocalPlayer::force_attack(DWORD attack)
{
    mem.write<DWORD>(mem.client_dll.get_image_base() + offsets.dwForceAttack, attack);
}

void LocalPlayer::set_fov(int fov)
{
    mem.write<int>(m_dwBaseAddr + offsets.m_iFOV, fov);
}
