#pragma once

class QAngle;
class Vector;

class BasePlayer
{
public:
    DWORD m_dwBaseAddr;

    BasePlayer();
    BasePlayer(int ent_index);

public:
    operator bool() const { return m_dwBaseAddr != 0; }

    int    m_iTeamNum();
    int    m_iHealth();
    int    m_iGlowIndex();
    int    m_iCrosshairId();
    int    m_iShotsFired();
    bool   m_bDormant();
    bool   m_bGunGameImmunity();
    bool   m_bIsScoped();
    BYTE   m_fFlags();
    QAngle m_aimPunchAngle();
    Vector m_vecOrigin();
    Vector m_vecVelocity();
    Vector m_vecViewOffset();
    Vector GetEyePos();
    Vector GetBonePos(int bone);
};

class LocalPlayer : public BasePlayer
{
public:
    LocalPlayer();

public:
    void Update();

    QAngle GetViewAngles();
    int EntIndex();
    int m_iFOV();

    void SetViewAngles(const QAngle &angles);
    void set_fov(int fov);
    void force_jump(DWORD jump);
    void force_attack(DWORD attack);

private:
    DWORD m_dwClientState;

};