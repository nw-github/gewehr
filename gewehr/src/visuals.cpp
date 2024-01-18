#include <stdafx.h>

#include "features.hpp"

#include "utils/memory.hpp"
#include "utils/offsets.hpp"
#include "utils/math.hpp"
#include "utils/entity.hpp"

using namespace std::chrono_literals;

namespace {
    void apply_chams(const Game &game, BasePlayer& player, LocalPlayer& local) {
        const auto &[mem, _, options] = game;
        if (player.m_iTeamNum() != local.m_iTeamNum()) {
            mem.write<BYTE>(player.m_dwBaseAddr + 0x70, options.chams_enemy_color.r);
            mem.write<BYTE>(player.m_dwBaseAddr + 0x71, options.chams_enemy_color.g);
            mem.write<BYTE>(player.m_dwBaseAddr + 0x72, options.chams_enemy_color.b);
        } else if (options.chams_teammates) {
            mem.write<BYTE>(player.m_dwBaseAddr + 0x70, options.chams_team_color.r);
            mem.write<BYTE>(player.m_dwBaseAddr + 0x71, options.chams_team_color.g);
            mem.write<BYTE>(player.m_dwBaseAddr + 0x72, options.chams_team_color.b);
        }
    }

    void apply_glow(const Game &game, BasePlayer& player, LocalPlayer& local, DWORD dwGlowObjManager) {
        const auto &[mem, _, options] = game;
        DWORD offset = dwGlowObjManager + (player.m_iGlowIndex() * sizeof(GlowObjectDefinition));
        auto obj = mem.read<GlowObjectDefinition>(offset);
        if (player.m_iTeamNum() != local.m_iTeamNum()) {
            obj.m_flRed = options.glow_enemy_color.r / 255.f;
            obj.m_flGreen = options.glow_enemy_color.g / 255.f;
            obj.m_flBlue = options.glow_enemy_color.b / 255.f;
            obj.m_flAlpha = options.glow_enemy_color.a / 255.f;
            obj.m_bRenderWhenOccluded = true;
            obj.m_bRenderWhenUnoccluded = false;
        } else if (options.glow_teammates) {
            obj.m_flRed = options.glow_team_color.r / 255.f;
            obj.m_flGreen = options.glow_team_color.g / 255.f;
            obj.m_flBlue = options.glow_team_color.b / 255.f;
            obj.m_flAlpha = options.glow_team_color.a / 255.f;
            obj.m_bRenderWhenOccluded = true;
            obj.m_bRenderWhenUnoccluded = false;
        }
        mem.write(offset, obj);
    }

    void set_chams_brightness(const Game &game) {
        const auto &[mem, offsets, options] = game;
        DWORD self = (DWORD)(mem.engine_dll.get_image_base() + offsets.modelAmbientMin - 0x2c);
        DWORD xored = *(DWORD*)&options.chams_brightness ^ self;
        mem.write<DWORD>(mem.engine_dll.get_image_base() + offsets.modelAmbientMin, xored);
    }
}

void visuals::thread_proc(std::stop_token token, const Game &game) {
    const auto &[mem, offsets, options] = game;

    SkinChanger skins(options);
    while (!token.stop_requested()) {
        std::this_thread::sleep_for(1ms);

        LocalPlayer local(mem, offsets);
        if (!local)
            continue;

        skins.tick(token, game);

        DWORD dwGlowObjManager = mem.read<DWORD>(mem.client_dll.get_image_base() + offsets.dwGlowObjManager);
        for (int i = 1; i < 64; i++) {
            BasePlayer player(mem, offsets, i);
            if (!player)
                continue;
            if (player.m_iHealth() <= 0 || player.m_iTeamNum() == 0)
                continue;
            if (player.m_bDormant() || i == local.EntIndex())
                continue;

            if (options.chams_enabled)
                apply_chams(game, player, local);

            if (options.glow_enabled)
                apply_glow(game, player, local, dwGlowObjManager);
        }

        if (options.chams_enabled)
            set_chams_brightness(game);

        if (options.override_fov)
            while (local.m_iFOV() != options.fov)
                local.set_fov(options.fov);
    }
}
