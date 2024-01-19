#include <stdafx.h>

#include "features.hpp"

#include "entity.hpp"
#include "math.hpp"
#include "utils.hpp"

using namespace std::chrono_literals;

void player::tbot_thread_proc(std::stop_token token, const State &s) {
    while (!token.stop_requested()) {
        std::this_thread::sleep_for(1ms);
        if (!s.has_focus() || (s.cfg.trigger_on_key && !utl::is_key_down(s.cfg.trigger_key))) {
            continue;
        }

        LocalPlayer local(s.mem, s.offsets);
        if (!local) {
            continue;
        }

        int player_idx = local.m_iCrosshairId() - 1;
        if (player_idx <= 0 || player_idx >= 64) {
            continue;
        }

        Player pl(s.mem, s.offsets, player_idx);
        if (!pl || pl.m_bDormant() || pl.m_bGunGameImmunity() || !pl.is_enemy_of(local)) {
            continue;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds{s.cfg.trigger_delay});
        local.force_attack(6);
    }
}

void player::bhop_thread_proc(std::stop_token token, const State &s) {
    while (!token.stop_requested()) {
        std::this_thread::sleep_for(1ms);
        if (!s.has_focus() || !utl::is_key_down(VK_SPACE)) {
            continue;
        }

        LocalPlayer local(s.mem, s.offsets);
        if (!local || !(local.m_fFlags() & FL_ONGROUND)) {
            continue;
        }

        local.force_jump(6);
    }
}

void player::rcs_thread_proc(std::stop_token token, const State &s) {
    QAngle old_punch(0.f, 0.f, 0.f);
    while (!token.stop_requested()) {
        std::this_thread::sleep_for(1ms);

        LocalPlayer local(s.mem, s.offsets);
        if (s.has_focus() && local && local.m_iShotsFired() > s.cfg.rcs_after_shots) {
            QAngle aim_punch = local.m_aimPunchAngle() * 2.f;
            aim_punch.pitch *= s.cfg.rcs_strength_y;
            aim_punch.yaw *= s.cfg.rcs_strength_x;
            aim_punch.roll = 0.f;
            m::normalize3(aim_punch);

            local.SetViewAngles(local.GetViewAngles() + (old_punch - aim_punch));

            old_punch = aim_punch;
        } else {
            old_punch.Init();
        }
    }
}
