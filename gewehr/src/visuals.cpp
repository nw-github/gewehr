#include <stdafx.h>

#include "features.hpp"

#include "utils/memory.hpp"
#include "utils/offsets.hpp"
#include "utils/options.hpp"
#include "utils/math.hpp"
#include "utils/render.hpp"
#include "utils/entity.hpp"

namespace {
    void apply_chams(BasePlayer& player, LocalPlayer& local) {
        int iEntTeam = player.m_iTeamNum();
        if (iEntTeam != local.m_iTeamNum()) {
            g::memory->write<BYTE>(player.m_dwBaseAddr + 0x70, g::options->chams_enemy_color.r);
            g::memory->write<BYTE>(player.m_dwBaseAddr + 0x71, g::options->chams_enemy_color.g);
            g::memory->write<BYTE>(player.m_dwBaseAddr + 0x72, g::options->chams_enemy_color.b);
        }
        else if (g::options->chams_teammates) {
            g::memory->write<BYTE>(player.m_dwBaseAddr + 0x70, g::options->chams_team_color.r);
            g::memory->write<BYTE>(player.m_dwBaseAddr + 0x71, g::options->chams_team_color.g);
            g::memory->write<BYTE>(player.m_dwBaseAddr + 0x72, g::options->chams_team_color.b);
        }
    }

    void apply_glow(BasePlayer& player, LocalPlayer& local, DWORD dwGlowObjManager) {
        int iGlowIndex = player.m_iGlowIndex();
        if (player.m_iTeamNum() != local.m_iTeamNum()) {
            GlowObjectDefinition obj = g::memory->read<GlowObjectDefinition>(dwGlowObjManager + (iGlowIndex * 0x38));
            obj.m_flRed = g::options->glow_enemy_color.r / 255.f;
            obj.m_flGreen = g::options->glow_enemy_color.g / 255.f;
            obj.m_flBlue = g::options->glow_enemy_color.b / 255.f;
            obj.m_flAlpha = g::options->glow_enemy_color.a / 255.f;
            obj.m_bRenderWhenOccluded = true;
            obj.m_bRenderWhenUnoccluded = false;
            g::memory->write(dwGlowObjManager + (iGlowIndex * 0x38), obj);
        }
        else if (g::options->glow_teammates) {
            GlowObjectDefinition obj = g::memory->read<GlowObjectDefinition>(dwGlowObjManager + (iGlowIndex * 0x38));
            obj.m_flRed = g::options->glow_team_color.r / 255.f;
            obj.m_flGreen = g::options->glow_team_color.g / 255.f;
            obj.m_flBlue = g::options->glow_team_color.b / 255.f;
            obj.m_flAlpha = g::options->glow_team_color.a / 255.f;
            obj.m_bRenderWhenOccluded = true;
            obj.m_bRenderWhenUnoccluded = false;
            g::memory->write(dwGlowObjManager + (iGlowIndex * 0x38), obj);
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

    void set_chams_brightness() {
        DWORD self = (DWORD)(g::memory->engine_dll.get_image_base() + g::offsets->modelAmbientMin - 0x2c);
        DWORD xored = *(DWORD*)&g::options->chams_brightness ^ self;
        g::memory->write<DWORD>(g::memory->engine_dll.get_image_base() + g::offsets->modelAmbientMin, xored);
    }
}

void visuals::thread_proc(std::stop_token token) {
    LocalPlayer local;
    while (!token.stop_requested()) {
        local.Update();
        if (!local)
            continue;

        DWORD dwGlowObjManager = g::memory->read<DWORD>(g::memory->client_dll.get_image_base() + g::offsets->dwGlowObjManager);
        // matrix4x4_t view_matrix = g::memory->read<matrix4x4_t>(g::memory->client_dll.get_image_base() + g::offsets->dwViewMatrix);
        for (int i = 1; i < 64; i++) {
            BasePlayer player(i);
            if (!player)
                continue;
            if (player.m_iHealth() <= 0 || player.m_iTeamNum() == 0)
                continue;
            if (player.m_bDormant() || i == local.EntIndex())
                continue;

            if (g::options->chams_enabled)
                apply_chams(player, local);

            if (g::options->glow_enabled)
                apply_glow(player, local, dwGlowObjManager);

//             if (g::options->esp_enabled)
//                 render_esp(player);
        }

        if (g::options->chams_enabled)
            set_chams_brightness();

        if (g::options->override_fov)
            while (local.m_iFOV() != g::options->fov)
                local.set_fov(g::options->fov);
    }
}
