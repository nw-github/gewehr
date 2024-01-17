#pragma once

// credits: xSkins [https://github.com/0xf1a/xSkins]

struct WeaponSkin {
    std::string name;
    SHORT       id{0};
    UINT        kit{0};
    float       wear{0.0001f};

public:
    WeaponSkin() = default;

    WeaponSkin(const char* name, SHORT weapon_id, UINT paint_kit, float fallback_wear)
        : name{ name }, id{ weapon_id }
        , kit{ paint_kit }, wear{ fallback_wear }
    { }

};

class SkinChanger {
public:
    void thread_proc(std::stop_token token);

private:
    static WeaponSkin get_weapon_skin(const short item_idx);

    static constexpr short knife_ids[19] = {
        WEAPON_KNIFE_BAYONET,
        WEAPON_KNIFE_FLIP,
        WEAPON_KNIFE_GUT,
        WEAPON_KNIFE_KARAMBIT,
        WEAPON_KNIFE_M9_BAYONET,
        WEAPON_KNIFE_TACTICAL,
        WEAPON_KNIFE_FALCHION,
        WEAPON_KNIFE_SURVIVAL_BOWIE,
        WEAPON_KNIFE_BUTTERFLY,
        WEAPON_KNIFE_PUSH,
        WEAPON_KNIFE_URSUS,
        WEAPON_KNIFE_GYPSY_JACKKNIFE,
        WEAPON_KNIFE_STILETTO,
        WEAPON_KNIFE_WIDOWMAKER,
        WEAPON_KNIFE_CSS,
        WEAPON_KNIFE_CORD,
        WEAPON_KNIFE_CANIS,
        WEAPON_KNIFE_OUTDOOR,
        WEAPON_KNIFE_SKELETON
    };
    
};

namespace f {
    inline std::unique_ptr<SkinChanger> skins = std::make_unique<SkinChanger>();
}