#include <stdafx.h>

#include "utils/entity.hpp"

#include "utils/memory.hpp"
#include "utils/offsets.hpp"

Player::Player(const Memory &mem, const Offsets &offsets, int idx)
    : mem(mem), offsets(offsets), baseAddr(mem.read<DWORD>(offsets.dwEntityList + (idx * 0x10))) { }

int Player::m_iTeamNum() {
    return mem.read<int>(baseAddr + offsets.m_iTeamNum);
}

int Player::m_iHealth() {
    return mem.read<int>(baseAddr + offsets.m_iHealth);
}

int Player::m_iGlowIndex() {
    return mem.read<int>(baseAddr + offsets.m_iGlowIndex);
}

int Player::m_iCrosshairId() {
    return mem.read<int>(baseAddr + offsets.m_iCrosshairId);
}

int Player::m_iShotsFired() {
    return mem.read<int>(baseAddr + offsets.m_iShotsFired);
}

bool Player::m_bDormant() {
    return mem.read<bool>(baseAddr + offsets.bDormant);
}

bool Player::m_bGunGameImmunity() {
    return mem.read<bool>(baseAddr + offsets.m_bGunGameImmunity);
}

bool Player::m_bIsScoped() {
    return mem.read<bool>(baseAddr + offsets.m_bIsScoped);
}

BYTE Player::m_fFlags() {
    return mem.read<BYTE>(baseAddr + offsets.m_fFlags);
}

QAngle Player::m_aimPunchAngle() {
    return mem.read<QAngle>(baseAddr + offsets.m_aimPunchAngle);
}

Vector Player::m_vecOrigin() {
    return mem.read<Vector>(baseAddr + offsets.m_vecOrigin);
}

Vector Player::m_vecVelocity() {
    return mem.read<Vector>(baseAddr + offsets.m_vecVelocity);
}

Vector Player::m_vecViewOffset() {
    return mem.read<Vector>(baseAddr + offsets.m_vecViewOffset);
}

Vector Player::GetBonePos(int bone) {
    auto boneMatrix = mem.read<DWORD>(baseAddr + offsets.m_dwBoneMatrix);
    auto matrix     = mem.read<matrix3x4_t>(boneMatrix + bone * sizeof(matrix3x4_t));
    return {matrix[0][3], matrix[1][3], matrix[2][3]};
}

Vector Player::GetEyePos() {
    Vector eyePos = m_vecOrigin();
    eyePos.z += mem.read<Vector>(baseAddr + offsets.m_vecViewOffset).z;
    return eyePos;
}

//------------------------------------------

LocalPlayer::LocalPlayer(const Memory &mem, const Offsets &offsets) : Player(mem, offsets) {
    baseAddr      = mem.read<DWORD>(offsets.dwLocalPlayer);
    dwClientState = mem.read<DWORD>(offsets.dwClientState);
}

QAngle LocalPlayer::GetViewAngles() {
    return mem.read<QAngle>(dwClientState + offsets.dwClientState_ViewAngles);
}

int LocalPlayer::EntIndex() {
    return mem.read<int>(dwClientState + offsets.dwClientState_GetLocalPlayer) + 1;
}

int LocalPlayer::m_iFOV() {
    return mem.read<int>(baseAddr + offsets.m_iFOV);
}

void LocalPlayer::SetViewAngles(const QAngle &angles) {
    mem.write(dwClientState + offsets.dwClientState_ViewAngles, angles);
}

void LocalPlayer::force_jump(DWORD jump) {
    mem.write(offsets.dwForceJump, jump);
}

void LocalPlayer::force_attack(DWORD attack) {
    mem.write(offsets.dwForceAttack, attack);
}

void LocalPlayer::set_fov(int fov) {
    mem.write(baseAddr + offsets.m_iFOV, fov);
}
