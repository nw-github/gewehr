#include <stdafx.h>

#include "features.hpp"

#include "utils/memory.hpp"
#include "utils/offsets.hpp"
#include "utils/math.hpp"
#include "utils/render.hpp"
#include "utils/entity.hpp"

using namespace std::chrono_literals;

namespace {
    void apply_chams(const Game &game, BasePlayer& player, LocalPlayer& local) {
        const auto &[mem, _, options] = game;

        int iEntTeam = player.m_iTeamNum();
        if (iEntTeam != local.m_iTeamNum()) {
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

        int iGlowIndex = player.m_iGlowIndex();
        if (player.m_iTeamNum() != local.m_iTeamNum()) {
            GlowObjectDefinition obj = mem.read<GlowObjectDefinition>(dwGlowObjManager + (iGlowIndex * 0x38));
            obj.m_flRed = options.glow_enemy_color.r / 255.f;
            obj.m_flGreen = options.glow_enemy_color.g / 255.f;
            obj.m_flBlue = options.glow_enemy_color.b / 255.f;
            obj.m_flAlpha = options.glow_enemy_color.a / 255.f;
            obj.m_bRenderWhenOccluded = true;
            obj.m_bRenderWhenUnoccluded = false;
            mem.write(dwGlowObjManager + (iGlowIndex * 0x38), obj);
        } else if (options.glow_teammates) {
            GlowObjectDefinition obj = mem.read<GlowObjectDefinition>(dwGlowObjManager + (iGlowIndex * 0x38));
            obj.m_flRed = options.glow_team_color.r / 255.f;
            obj.m_flGreen = options.glow_team_color.g / 255.f;
            obj.m_flBlue = options.glow_team_color.b / 255.f;
            obj.m_flAlpha = options.glow_team_color.a / 255.f;
            obj.m_bRenderWhenOccluded = true;
            obj.m_bRenderWhenUnoccluded = false;
            mem.write(dwGlowObjManager + (iGlowIndex * 0x38), obj);
        }
    }

// void render_esp(BasePlayer &player) {
//    if (!u::is_csgo_focused())
//        return;
// 
//    Vector base, head;
//    if (!m::world_to_screen(player.m_vecOrigin(), base, m_wts_matrix))
//        return;
//    if (!m::world_to_screen(player.GetEyePos(), head, m_wts_matrix))
//        return;
// 
//    int h = head.y - base.y;
//    int w = h / 2;
// 
//    g::render->add_rect(base.x - w / 2, base.y, w, h, D3DCOLOR_ARGB(255, 0, 255, 0));
// }

    void set_chams_brightness(const Game &game) {
        const auto &[mem, offsets, options] = game;
        DWORD self = (DWORD)(mem.engine_dll.get_image_base() + offsets.modelAmbientMin - 0x2c);
        DWORD xored = *(DWORD*)&options.chams_brightness ^ self;
        mem.write<DWORD>(mem.engine_dll.get_image_base() + offsets.modelAmbientMin, xored);
    }
}

void visuals::thread_proc(std::stop_token token, const Game &game) {
    const auto &[mem, offsets, options] = game;
    while (!token.stop_requested()) {
        std::this_thread::sleep_for(1ms);

        LocalPlayer local(mem, offsets);
        if (!local)
            continue;

        DWORD dwGlowObjManager = mem.read<DWORD>(mem.client_dll.get_image_base() + offsets.dwGlowObjManager);
        // matrix4x4_t view_matrix = mem.read<matrix4x4_t>(mem.client_dll.get_image_base() + offsets.dwViewMatrix);
        for (int i = 1; i < 64; i++) {
            BasePlayer player(game.mem, game.offsets, i);
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

//             if (options.esp_enabled)
//                 render_esp(player);
        }

        if (options.chams_enabled)
            set_chams_brightness(game);

        if (options.override_fov)
            while (local.m_iFOV() != options.fov)
                local.set_fov(options.fov);
    }
}
