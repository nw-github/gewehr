#include <stdafx.h>

#include "features.hpp"

#include "utils/memory.hpp"
#include "utils/offsets.hpp"
#include "utils/math.hpp"
#include "utils/entity.hpp"

using namespace std::chrono_literals;

namespace
{
    void apply_chams(const State &s, BasePlayer &player, LocalPlayer &local)
    {
        const auto &[mem, _, cfg] = s;
        if (player.m_iTeamNum() != local.m_iTeamNum())
        {
            mem.write<BYTE>(player.m_dwBaseAddr + 0x70, cfg.chams_enemy_color.r);
            mem.write<BYTE>(player.m_dwBaseAddr + 0x71, cfg.chams_enemy_color.g);
            mem.write<BYTE>(player.m_dwBaseAddr + 0x72, cfg.chams_enemy_color.b);
        }
        else if (cfg.chams_teammates)
        {
            mem.write<BYTE>(player.m_dwBaseAddr + 0x70, cfg.chams_team_color.r);
            mem.write<BYTE>(player.m_dwBaseAddr + 0x71, cfg.chams_team_color.g);
            mem.write<BYTE>(player.m_dwBaseAddr + 0x72, cfg.chams_team_color.b);
        }
    }

    void apply_glow(const State &s, BasePlayer &player, LocalPlayer &local, DWORD dwGlowObjManager)
    {
        const auto &[mem, _, cfg] = s;
        DWORD offset = dwGlowObjManager + (player.m_iGlowIndex() * sizeof(GlowObjectDefinition));
        auto obj = mem.read<GlowObjectDefinition>(offset);
        if (player.m_iTeamNum() != local.m_iTeamNum())
        {
            obj.m_flRed = cfg.glow_enemy_color.r / 255.f;
            obj.m_flGreen = cfg.glow_enemy_color.g / 255.f;
            obj.m_flBlue = cfg.glow_enemy_color.b / 255.f;
            obj.m_flAlpha = cfg.glow_enemy_color.a / 255.f;
            obj.m_bRenderWhenOccluded = true;
            obj.m_bRenderWhenUnoccluded = false;
        }
        else if (cfg.glow_teammates)
        {
            obj.m_flRed = cfg.glow_team_color.r / 255.f;
            obj.m_flGreen = cfg.glow_team_color.g / 255.f;
            obj.m_flBlue = cfg.glow_team_color.b / 255.f;
            obj.m_flAlpha = cfg.glow_team_color.a / 255.f;
            obj.m_bRenderWhenOccluded = true;
            obj.m_bRenderWhenUnoccluded = false;
        }
        mem.write(offset, obj);
    }

    void set_chams_brightness(const State &s)
    {
        const auto &[mem, offsets, cfg] = s;
        DWORD self = (DWORD)(mem.engine_dll.get_image_base() + offsets.modelAmbientMin - 0x2c);
        DWORD xored = *(DWORD *)&cfg.chams_brightness ^ self;
        mem.write<DWORD>(mem.engine_dll.get_image_base() + offsets.modelAmbientMin, xored);
    }
}

void visuals::thread_proc(std::stop_token token, const State &s)
{
    const auto &[mem, offsets, cfg] = s;

    SkinChanger skins(cfg);
    while (!token.stop_requested())
    {
        std::this_thread::sleep_for(1ms);

        LocalPlayer local(mem, offsets);
        if (!local)
            continue;

        skins.tick(token, s);

        DWORD dwGlowObjManager = mem.read<DWORD>(mem.client_dll.get_image_base() + offsets.dwGlowObjManager);
        for (int i = 1; i < 64; i++)
        {
            BasePlayer player(mem, offsets, i);
            if (!player)
                continue;
            if (player.m_iHealth() <= 0 || player.m_iTeamNum() == 0)
                continue;
            if (player.m_bDormant() || i == local.EntIndex())
                continue;

            if (cfg.chams_enabled)
                apply_chams(s, player, local);

            if (cfg.glow_enabled)
                apply_glow(s, player, local, dwGlowObjManager);
        }

        if (cfg.chams_enabled)
            set_chams_brightness(s);

        if (cfg.override_fov)
            while (local.m_iFOV() != cfg.fov)
                local.set_fov(cfg.fov);
    }
}
