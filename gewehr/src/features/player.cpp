#include <stdafx.h>

#include "features/player.hpp"

#include "utils/options.hpp"
#include "utils/utils.hpp"
#include "utils/math.hpp"
#include "utils/entity.hpp"

using namespace std::chrono_literals;

void Player::tbot_thread_proc(std::stop_token token) {
    while (!token.stop_requested()) {
        std::this_thread::sleep_for(1ms);

        if (!g::options->trigger_enabled || !utl::is_csgo_focused())
            continue;

        if (!g::options->trigger_on_key || utl::is_key_down(g::options->trigger_key)) {
            LocalPlayer local;
            if (!local)
                continue;

            int player_number = local.m_iCrosshairId() - 1;
            if (player_number > 0 && player_number < 64) {
                BasePlayer player(player_number);
                if (!player || player.m_bDormant() || player.m_bGunGameImmunity())
                    continue;
                if (player.m_iTeamNum() == local.m_iTeamNum())
                    continue;

                std::this_thread::sleep_for(std::chrono::milliseconds{g::options->trigger_delay});
                local.force_attack(6);
            }
        }
    }
}

void Player::bhop_thread_proc(std::stop_token token) {
    while (!token.stop_requested()) {
        std::this_thread::sleep_for(1ms);

        if (!g::options->bhop_enabled)
            continue;

        if (!utl::is_csgo_focused())
            continue;

        LocalPlayer local;
        if (!local)
            continue;

        if (utl::is_key_down(VK_SPACE))
            if (local.m_fFlags() & FL_ONGROUND)
                local.force_jump(6);
    }
}

void Player::rcs_thread_proc(std::stop_token token) {
    QAngle old_punch(0.f, 0.f, 0.f);
    while (!token.stop_requested()) {
        if (!g::options->rcs_enabled || !utl::is_csgo_focused()) {
            old_punch.Init(0.f, 0.f, 0.f);
        } else {
            LocalPlayer local;
            if (local && local.m_iShotsFired() > g::options->rcs_after_shots) {
                QAngle aim_punch = local.m_aimPunchAngle() * 2.f;
                aim_punch.pitch *= g::options->rcs_strength_y;
                aim_punch.yaw *= g::options->rcs_strength_x;
                aim_punch.roll = 0.f;
                m::normalize3(aim_punch);

                local.SetViewAngles(local.GetViewAngles() + (old_punch - aim_punch));

                old_punch = aim_punch;
            } else {
                old_punch.Init(0.f, 0.f, 0.f);
            }
        }

        std::this_thread::sleep_for(1ms);
    }
}