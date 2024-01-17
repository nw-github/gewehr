#include <stdafx.h>

#include "utils/entity.hpp"

#include "utils/memory.hpp"
#include "utils/offsets.hpp"

BasePlayer::BasePlayer() : m_dwBaseAddr{0} { }

BasePlayer::BasePlayer(int ent_index)
{
    m_dwBaseAddr = g::memory->read<DWORD>(
        g::memory->client_dll.get_image_base() + g::offsets->dwEntityList + (ent_index * 0x10)
    );
}

int BasePlayer::m_iTeamNum()
{
    return g::memory->read<int>(m_dwBaseAddr + g::offsets->m_iTeamNum);
}

int BasePlayer::m_iHealth()
{
    return g::memory->read<int>(m_dwBaseAddr + g::offsets->m_iHealth);
}

int BasePlayer::m_iGlowIndex()
{
    return g::memory->read<int>(m_dwBaseAddr + g::offsets->m_iGlowIndex);
}

int BasePlayer::m_iCrosshairId()
{
    return g::memory->read<int>(m_dwBaseAddr + g::offsets->m_iCrosshairId);
}

int BasePlayer::m_iShotsFired()
{
    return g::memory->read<int>(m_dwBaseAddr + g::offsets->m_iShotsFired);
}

bool BasePlayer::m_bDormant()
{
    return g::memory->read<bool>(m_dwBaseAddr + g::offsets->bDormant);
}

bool BasePlayer::m_bGunGameImmunity()
{
    return g::memory->read<bool>(m_dwBaseAddr + g::offsets->m_bGunGameImmunity);
}

bool BasePlayer::m_bIsScoped()
{
    return g::memory->read<bool>(m_dwBaseAddr + g::offsets->m_bIsScoped);
}

BYTE BasePlayer::m_fFlags()
{
    return g::memory->read<BYTE>(m_dwBaseAddr + g::offsets->m_fFlags);
}

QAngle BasePlayer::m_aimPunchAngle()
{
    return g::memory->read<QAngle>(m_dwBaseAddr + g::offsets->m_aimPunchAngle);
}

Vector BasePlayer::m_vecOrigin()
{
    return g::memory->read<Vector>(m_dwBaseAddr + g::offsets->m_vecOrigin);
}

Vector BasePlayer::m_vecVelocity()
{
    return g::memory->read<Vector>(m_dwBaseAddr + g::offsets->m_vecVelocity);
}

Vector BasePlayer::m_vecViewOffset()
{
    return g::memory->read<Vector>(m_dwBaseAddr + g::offsets->m_vecViewOffset);
}

Vector BasePlayer::GetBonePos(int bone)
{
    Vector pos;
    DWORD boneMatrix = g::memory->read<DWORD>(m_dwBaseAddr + g::offsets->m_dwBoneMatrix);

    pos.x = g::memory->read<float>(boneMatrix + (0x30 * bone) + 0x0C);
    pos.y = g::memory->read<float>(boneMatrix + (0x30 * bone) + 0x1C);
    pos.z = g::memory->read<float>(boneMatrix + (0x30 * bone) + 0x2C);
    return pos;
}

Vector BasePlayer::GetEyePos()
{
    Vector eyePos = m_vecOrigin();
    eyePos.z += g::memory->read<Vector>(m_dwBaseAddr + g::offsets->m_vecViewOffset).z;
    return eyePos;
}

//------------------------------------------

LocalPlayer::LocalPlayer()
{
    m_dwClientState = g::memory->read<DWORD>(g::offsets->dwClientState);
    Update();
}

void LocalPlayer::Update()
{
    m_dwBaseAddr = g::memory->read<DWORD>(g::offsets->dwLocalPlayer);
}

QAngle LocalPlayer::GetViewAngles()
{
    return g::memory->read<QAngle>(m_dwClientState + g::offsets->dwClientState_ViewAngles);
}

int LocalPlayer::EntIndex()
{
    return g::memory->read<int>(m_dwClientState + g::offsets->dwClientState_GetLocalPlayer) + 1;
}

int LocalPlayer::m_iFOV()
{
    return g::memory->read<int>(m_dwBaseAddr + g::offsets->m_iFOV);
}

void LocalPlayer::SetViewAngles(const QAngle &angles)
{
    g::memory->write(m_dwClientState + g::offsets->dwClientState_ViewAngles, angles);
}

void LocalPlayer::force_jump(DWORD jump)
{
    g::memory->write<DWORD>(g::memory->client_dll.get_image_base() + g::offsets->dwForceJump, jump);
}

void LocalPlayer::force_attack(DWORD attack)
{
    g::memory->write<DWORD>(g::memory->client_dll.get_image_base() + g::offsets->dwForceAttack, attack);
}

void LocalPlayer::set_fov(int fov)
{
    g::memory->write<int>(m_dwBaseAddr + g::offsets->m_iFOV, fov);
}