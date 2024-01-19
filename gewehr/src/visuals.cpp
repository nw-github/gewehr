#include <stdafx.h>

#include "features.hpp"

#include "utils/entity.hpp"
#include "utils/math.hpp"
#include "utils/memory.hpp"
#include "utils/offsets.hpp"

using namespace std::chrono_literals;

namespace {
    void apply_chams(const State &s, Player &player, LocalPlayer &local) {
        const auto is_teammate = !player.is_enemy_of(local);
        if (is_teammate && !s.cfg.chams_teammates) {
            return;
        }

        const auto &color = is_teammate ? s.cfg.chams_team_color : s.cfg.chams_enemy_color;
        s.mem.write(player.baseAddr + 0x70, std::array<BYTE, 3>{color.r, color.g, color.b});
    }

    void apply_glow(const State &s, Player &player, LocalPlayer &local, DWORD glowObjManager) {
        const auto is_teammate = !player.is_enemy_of(local);
        if (is_teammate && !s.cfg.glow_teammates) {
            return;
        }
        const auto &color = is_teammate ? s.cfg.glow_team_color : s.cfg.glow_enemy_color;
        DWORD offset      = glowObjManager + (player.m_iGlowIndex() * sizeof(GlowObjectDefinition));
        auto obj          = s.mem.read<GlowObjectDefinition>(offset);
        obj.m_flRed       = color.r / 255.f;
        obj.m_flGreen     = color.g / 255.f;
        obj.m_flBlue      = color.b / 255.f;
        obj.m_flAlpha     = color.a / 255.f;
        obj.m_bRenderWhenOccluded   = true;
        obj.m_bRenderWhenUnoccluded = false;
        s.mem.write(offset, obj);
    }

    void set_chams_brightness(const State &s) {
        DWORD self  = s.offsets.modelAmbientMin - 0x2c;
        DWORD xored = std::bit_cast<DWORD>(s.cfg.chams_brightness) ^ self;
        s.mem.write<DWORD>(s.offsets.modelAmbientMin, xored);
    }
} // namespace

void visuals::thread_proc(std::stop_token token, const State &s) {
    const auto &[mem, offsets, cfg] = s;

    SkinChanger skins(cfg);
    while (!token.stop_requested()) {
        skins.tick(token, s);

        LocalPlayer local(mem, offsets);
        if (!local) {
            continue;
        }

        if (cfg.override_fov) {
            while (local.m_iFOV() != cfg.fov) {
                local.set_fov(cfg.fov);
            }
        }

        if (!cfg.chams_enabled && !cfg.glow_enabled) {
            continue;
        }

        auto glowObjManager = mem.read<DWORD>(offsets.dwGlowObjManager);
        for (int i = 1; i < 64; i++) {
            Player pl(mem, offsets, i);
            if (!pl || pl.m_iHealth() <= 0 || pl.m_iTeamNum() == 0 || pl.m_bDormant() ||
                pl.baseAddr == local.baseAddr) {
                continue;
            }

            if (cfg.chams_enabled) {
                apply_chams(s, pl, local);
            }
            if (cfg.glow_enabled) {
                apply_glow(s, pl, local, glowObjManager);
            }
        }

        if (cfg.chams_enabled) {
            set_chams_brightness(s);
        }
    }
}
