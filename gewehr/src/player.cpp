#include <stdafx.h>

#include "features.hpp"

#include "utils/utils.hpp"
#include "utils/math.hpp"
#include "utils/entity.hpp"

using namespace std::chrono_literals;

void player::tbot_thread_proc(std::stop_token token, const State &s)
{
    const auto &cfg = s.cfg;
    while (!token.stop_requested())
    {
        std::this_thread::sleep_for(1ms);
        if (!utl::is_csgo_focused(s.mem))
            continue;

        if (!cfg.trigger_on_key || utl::is_key_down(cfg.trigger_key))
        {
            LocalPlayer local(s.mem, s.offsets);
            if (!local)
                continue;

            int player_number = local.m_iCrosshairId() - 1;
            if (player_number <= 0 || player_number >= 64)
                continue;

            BasePlayer player(s.mem, s.offsets, player_number);
            if (!player || player.m_bDormant() || player.m_bGunGameImmunity())
                continue;
            if (player.m_iTeamNum() == local.m_iTeamNum())
                continue;

            std::this_thread::sleep_for(std::chrono::milliseconds{cfg.trigger_delay});
            local.force_attack(6);
        }
    }
}

void player::bhop_thread_proc(std::stop_token token, const State &s)
{
    while (!token.stop_requested())
    {
        std::this_thread::sleep_for(1ms);
        if (!utl::is_csgo_focused(s.mem))
            continue;

        LocalPlayer local(s.mem, s.offsets);
        if (!local)
            continue;

        if (utl::is_key_down(VK_SPACE))
            if (local.m_fFlags() & FL_ONGROUND)
                local.force_jump(6);
    }
}

void player::rcs_thread_proc(std::stop_token token, const State &s)
{
    QAngle old_punch(0.f, 0.f, 0.f);

    const auto &cfg = s.cfg;
    while (!token.stop_requested())
    {
        LocalPlayer local(s.mem, s.offsets);
        if (utl::is_csgo_focused(s.mem) &&
            local &&
            local.m_iShotsFired() > cfg.rcs_after_shots)
        {
            QAngle aim_punch = local.m_aimPunchAngle() * 2.f;
            aim_punch.pitch *= cfg.rcs_strength_y;
            aim_punch.yaw *= cfg.rcs_strength_x;
            aim_punch.roll = 0.f;
            m::normalize3(aim_punch);

            local.SetViewAngles(local.GetViewAngles() + (old_punch - aim_punch));

            old_punch = aim_punch;
        }
        else
        {
            old_punch.Init();
        }

        std::this_thread::sleep_for(1ms);
    }
}
