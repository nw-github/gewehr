#pragma once

#include "config.hpp"

#define EXPAND_WEAPON(id, kit) x, WeaponSkin(xorstr(#id), id, kit, 0.0001f)

#define ADD_OPTION(ty, name, def)        ty &name = g::config->add_option(new Option<ty>(xorstr(#name), def))
#define ADD_BRACE_OPTION(ty, name, ...) ty &name = g::config->add_option(new Option<ty>(xorstr(#name), { __VA_ARGS__ }))

struct Options {
    // Global settings
    ADD_OPTION(SHORT, exit_key, VK_END);
    ADD_OPTION(SHORT, refresh_cfg_key, VK_F10);
    // Toggle Keys
    ADD_OPTION(SHORT, bhop_toggle_key, 0);       // Toggle bhop enabled
    ADD_OPTION(SHORT, rcs_toggle_key, 0);        // Toggle RCS enabled
    ADD_OPTION(SHORT, trigger_toggle_key, 0);    // Toggle triggerbot enabled
    ADD_OPTION(SHORT, visuals_toggle_key, VK_F6);
    ADD_OPTION(SHORT, esp_toggle_key, 0);
    // Toggleable 
    ADD_OPTION(bool, bhop_enabled, true);        // Is bhop enabled
    ADD_OPTION(bool, rcs_enabled, false);        // Is RCS enabled
    ADD_OPTION(bool, trigger_enabled, true);     // Is triggerbot enabled
    ADD_OPTION(bool, visuals_enabled, false);
    ADD_OPTION(bool, esp_enabled, false);
    // 
    ADD_OPTION(bool, glow_enabled, false);
    ADD_OPTION(bool, chams_enabled, false);
    ADD_OPTION(bool, skins_enabled, true);
    // Skinchanger settings
    ADD_OPTION(short, skins_knife_id, 0);
    ADD_OPTION(UINT, skins_knife_skin_id, 0);
    std::map<SHORT, WeaponSkin> &skin_map = 
        g::config->add_option(new Option<std::map<SHORT, WeaponSkin>>(xorstr("skin_map"), {}));
    // RCS settings
    ADD_OPTION(float, rcs_strength_x, 0.4f);    // RCS pull strength X
    ADD_OPTION(float, rcs_strength_y, 0.2f);    // RCS pull strength Y
    ADD_OPTION(int, rcs_after_shots, 1);        // RCS after X bullets
    // Triggerbot settings
    ADD_OPTION(bool, trigger_on_key, true);     // Should triggerbot only when key is pressed
    ADD_OPTION(SHORT, trigger_key, VK_LMENU);   // Key to press to enable triggerbot
    ADD_OPTION(UINT, trigger_delay, 2);         // Delay before shooting
    // glow settings
    ADD_OPTION(bool, glow_teammates, false);
    ADD_BRACE_OPTION(Color, glow_enemy_color, 255, 128, 0, 200);
    ADD_BRACE_OPTION(Color, glow_team_color, 0, 255, 255, 200);
    // chams settings
    ADD_OPTION(bool, chams_teammates, false);
    ADD_BRACE_OPTION(Color, chams_enemy_color, 255, 128, 0, 255);
    ADD_BRACE_OPTION(Color, chams_team_color, 0, 255, 255, 255);
    ADD_OPTION(float, chams_brightness, 5.f);
    // Visual settings
    ADD_OPTION(bool, override_fov, false);
    ADD_OPTION(bool, override_view, false);
    ADD_OPTION(int, fov, 90);
    ADD_OPTION(int, viewmodel_fov, 60);

};

namespace g
{
    inline std::unique_ptr<::Options> options = std::make_unique<::Options>();
}