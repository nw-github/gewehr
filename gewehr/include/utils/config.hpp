#pragma once

#pragma warning(push, 0)
#include <nlohmann/json.hpp>
#pragma warning(pop)

#include "utils/utils.hpp"

#define FOLDER_NAME xorstr(".gw")

struct Color {
    BYTE r, g, b, a;

    Color(BYTE r = 0, BYTE g = 0, BYTE b = 0, BYTE a = 255)
        : r{r}, g{g}, b{b}, a{a}
    { }

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Color, r, g, b, a);
};

struct WeaponSkin {
    std::string name;
    SHORT id{0};
    UINT kit{0};
    float wear{0.0001f};

public:
    WeaponSkin() = default;

    WeaponSkin(std::string &&name, SHORT weapon_id, UINT paint_kit, float fallback_wear)
        : name{std::forward<std::string>(name)}, id{weapon_id}, kit{paint_kit}, wear{fallback_wear}
    { }

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(WeaponSkin, name, id, kit, wear);
};


struct Config {
    // Global settings
    SHORT exit_key = VK_END;
    SHORT refresh_cfg_key = VK_F10;
    // Toggle Keys
    SHORT bhop_toggle_key = 0;    // Toggle bhop enabled
    SHORT rcs_toggle_key = 0;     // Toggle RCS enabled
    SHORT trigger_toggle_key = 0; // Toggle triggerbot enabled
    SHORT visuals_toggle_key = VK_F6;
    // Toggleable
    bool bhop_enabled = true;    // Is bhop enabled
    bool rcs_enabled = false;    // Is RCS enabled
    bool trigger_enabled = true; // Is triggerbot enabled
    bool visuals_enabled = false;
    //
    bool glow_enabled = false;
    bool chams_enabled = false;
    bool skins_enabled = true;
    // Skinchanger settings
    short skins_knife_id = 0;
    UINT skins_knife_skin_id = 0;
    std::unordered_map<SHORT, WeaponSkin> skin_map;
    // RCS settings
    float rcs_strength_x = 0.4f; // RCS pull strength X
    float rcs_strength_y = 0.2f; // RCS pull strength Y
    int rcs_after_shots = 1;     // RCS after X bullets
    // Triggerbot settings
    bool trigger_on_key = true;   // Should triggerbot only when key is pressed
    SHORT trigger_key = VK_LMENU; // Key to press to enable triggerbot
    UINT trigger_delay = 2;       // Delay before shooting
    // glow settings
    bool glow_teammates = false;
    Color glow_enemy_color = {255, 128, 0, 200};
    Color glow_team_color = {0, 255, 255, 200};
    // chams settings
    bool chams_teammates = false;
    Color chams_enemy_color = {255, 128, 0, 255};
    Color chams_team_color = {0, 255, 255, 255};
    float chams_brightness = 5.f;
    // Visual settings
    bool override_fov = false;
    bool override_view = false;
    int fov = 90;
    int viewmodel_fov = 60;

public:
    Config() = default;
    static std::optional<Config> load();
    bool save() const;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Config,
                                   exit_key,
                                   refresh_cfg_key,
                                   bhop_toggle_key,
                                   rcs_toggle_key,
                                   trigger_toggle_key,
                                   visuals_toggle_key,
                                   bhop_enabled,
                                   rcs_enabled,
                                   trigger_enabled,
                                   visuals_enabled,
                                   glow_enabled,
                                   chams_enabled,
                                   skins_enabled,
                                   skins_knife_id,
                                   skins_knife_skin_id,
                                   skin_map,
                                   rcs_strength_x,
                                   rcs_strength_y,
                                   rcs_after_shots,
                                   trigger_on_key,
                                   trigger_key,
                                   trigger_delay,
                                   glow_teammates,
                                   glow_enemy_color,
                                   glow_team_color,
                                   chams_teammates,
                                   chams_enemy_color,
                                   chams_team_color,
                                   chams_brightness,
                                   override_fov,
                                   override_view,
                                   fov,
                                   viewmodel_fov);
};
