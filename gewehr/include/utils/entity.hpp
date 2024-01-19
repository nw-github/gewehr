#pragma once

#include "utils/memory.hpp"
#include "utils/offsets.hpp"

class Player {
public:
    DWORD baseAddr;

    Player(const Memory &mem, const Offsets &offsets, int ent_index);

public:
    operator bool() const {
        return baseAddr != 0;
    }

    bool is_enemy_of(Player &rhs) {
        return m_iTeamNum() != rhs.m_iTeamNum();
    }

    int m_iTeamNum();
    int m_iHealth();
    int m_iGlowIndex();
    int m_iCrosshairId();
    int m_iShotsFired();
    bool m_bDormant();
    bool m_bGunGameImmunity();
    bool m_bIsScoped();
    BYTE m_fFlags();
    QAngle m_aimPunchAngle();
    Vector m_vecOrigin();
    Vector m_vecVelocity();
    Vector m_vecViewOffset();
    Vector GetEyePos();
    Vector GetBonePos(int bone);

protected:
    const Memory &mem;
    const Offsets &offsets;

protected:
    Player(const Memory &mem, const Offsets &offsets) : mem(mem), offsets(offsets), baseAddr(0) { }
};

class LocalPlayer : public Player {
public:
    LocalPlayer(const Memory &mem, const Offsets &offsets);

public:
    QAngle GetViewAngles();
    int EntIndex();
    int m_iFOV();

    void SetViewAngles(const QAngle &angles);
    void set_fov(int fov);
    void force_jump(DWORD jump);
    void force_attack(DWORD attack);

private:
    DWORD dwClientState;
};