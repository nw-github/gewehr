#include <stdafx.h>

#include "features.hpp"

#include "utils/memory.hpp"
#include "utils/offsets.hpp"

// credits: xSkins [https://github.com/0xf1a/xSkins]

namespace {
    constexpr const short KNIFE_IDS[19] = {
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
        WEAPON_KNIFE_SKELETON,
    };
    constexpr const int ITEM_ID_HIGH = -1;
    constexpr const int ENTITY_QUALITY = 3;

    UINT get_model_idx_by_name(const State &s, std::string_view name) {
        const auto &[mem, offs, _] = s;

        auto cstate = mem.read<DWORD>(offs.dwClientState);
        auto nst = mem.read<DWORD>(cstate + offs.m_dwModelPrecache); // CClientState + 0x529C -> INetworkStringTable* m_pModelPrecacheTable
        auto nsd = mem.read<DWORD>(nst + 0x40);                      // INetworkStringTable + 0x40 -> INetworkStringDict* m_pItems
        auto nsdi = mem.read<DWORD>(nsd + 0xC);                      // INetworkStringDict + 0xC -> void* m_pItems
        for (UINT i = 0; i < 1024; i++) {
            auto nsdi_i = mem.read<DWORD>(nsdi + 0xC + i * 0x34);
            auto str = mem.read<std::array<char, 128>>(nsdi_i);
            if (utl::icompare(str.data(), name)) {
                return i;
            }
        }
        return 0;
    }

    UINT get_model_idx(const State &s, const short item_idx) {
        switch (item_idx) {
        case WEAPON_KNIFE:
            return get_model_idx_by_name(s, xorstr("models/weapons/v_knife_default_ct.mdl"));
        case WEAPON_KNIFE_T:
            return get_model_idx_by_name(s, xorstr("models/weapons/v_knife_default_t.mdl"));
        case WEAPON_KNIFE_BAYONET:
            return get_model_idx_by_name(s, xorstr("models/weapons/v_knife_bayonet.mdl"));
        case WEAPON_KNIFE_FLIP:
            return get_model_idx_by_name(s, xorstr("models/weapons/v_knife_flip.mdl"));
        case WEAPON_KNIFE_GUT:
            return get_model_idx_by_name(s, xorstr("models/weapons/v_knife_gut.mdl"));
        case WEAPON_KNIFE_KARAMBIT:
            return get_model_idx_by_name(s, xorstr("models/weapons/v_knife_karam.mdl"));
        case WEAPON_KNIFE_M9_BAYONET:
            return get_model_idx_by_name(s, xorstr("models/weapons/v_knife_m9_bay.mdl"));
        case WEAPON_KNIFE_TACTICAL:
            return get_model_idx_by_name(s, xorstr("models/weapons/v_knife_tactical.mdl"));
        case WEAPON_KNIFE_FALCHION:
            return get_model_idx_by_name(s, xorstr("models/weapons/v_knife_falchion_advanced.mdl"));
        case WEAPON_KNIFE_SURVIVAL_BOWIE:
            return get_model_idx_by_name(s, xorstr("models/weapons/v_knife_survival_bowie.mdl"));
        case WEAPON_KNIFE_BUTTERFLY:
            return get_model_idx_by_name(s, xorstr("models/weapons/v_knife_butterfly.mdl"));
        case WEAPON_KNIFE_PUSH:
            return get_model_idx_by_name(s, xorstr("models/weapons/v_knife_push.mdl"));
        case WEAPON_KNIFE_URSUS:
            return get_model_idx_by_name(s, xorstr("models/weapons/v_knife_ursus.mdl"));
        case WEAPON_KNIFE_GYPSY_JACKKNIFE:
            return get_model_idx_by_name(s, xorstr("models/weapons/v_knife_gypsy_jackknife.mdl"));
        case WEAPON_KNIFE_STILETTO:
            return get_model_idx_by_name(s, xorstr("models/weapons/v_knife_stiletto.mdl"));
        case WEAPON_KNIFE_WIDOWMAKER:
            return get_model_idx_by_name(s, xorstr("models/weapons/v_knife_widowmaker.mdl"));
        case WEAPON_KNIFE_CSS:
            return get_model_idx_by_name(s, xorstr("models/weapons/v_knife_css.mdl"));
        case WEAPON_KNIFE_CORD:
            return get_model_idx_by_name(s, xorstr("models/weapons/v_knife_cord.mdl"));
        case WEAPON_KNIFE_CANIS:
            return get_model_idx_by_name(s, xorstr("models/weapons/v_knife_canis.mdl"));
        case WEAPON_KNIFE_OUTDOOR:
            return get_model_idx_by_name(s, xorstr("models/weapons/v_knife_outdoor.mdl"));
        case WEAPON_KNIFE_SKELETON:
            return get_model_idx_by_name(s, xorstr("models/weapons/v_knife_skeleton.mdl"));
        default:
            return 0;
        }
    }

    DWORD get_handle_entity(const Memory &mem, const Offsets &offs, DWORD addr) {
        auto handle = mem.read<DWORD>(addr) & 0xfff;
        return mem.read<DWORD>(offs.dwEntityList2 + (handle - 1) * 0x10);
    }
}

SkinChanger::SkinChanger(const Config &cfg)
    : model_index(0), local(0), last_knife_id(KNIFE_IDS[cfg.skins_knife_id])
{ }

void SkinChanger::tick(const std::stop_token &token, const State &s) {
    const auto &[mem, offs, cfg] = s;
    if (!cfg.skins_enabled) {
        return;
    }

    // model index is different for each server and map
    // below is a simple way to keep track of local base in order to reset model index
    // while also avoiding doing unnecessary extra reads because of the external RPM overhead
    auto tmp_player = mem.read<DWORD>(offs.dwLocalPlayer);
    if (!tmp_player) { // client not connected to any server (works most of the time)
        model_index = 0;
        return;
    } else if (tmp_player != local) { // local base changed (new server join/demo record)
        local = tmp_player;
        model_index = 0;
    }

    short knife_idx = KNIFE_IDS[cfg.skins_knife_id];
    if (last_knife_id != knife_idx) {
        model_index = 0;
        last_knife_id = knife_idx;
    }

    while (!model_index && !token.stop_requested()) {
        model_index = get_model_idx(s, knife_idx);
    }

    // loop through m_hMyWeapons slots (8 will be enough)
    for (size_t i = 0; i < 8; i++) {
        // get entity of weapon in current slot
        auto current_weapon = get_handle_entity(mem, offs, local + offs.m_hMyWeapons + i * 0x4);
        if (!current_weapon) {
            continue;
        }

        auto weapon_idx = mem.read<short>(current_weapon + offs.m_iItemDefinitionIndex);
        auto skin = cfg.skin_map.count(weapon_idx) 
            ? cfg.skin_map.at(weapon_idx) 
            : WeaponSkin("", weapon_idx, 0, 0.0001f);

        // for knives, set item and model related properties
        if (weapon_idx == WEAPON_KNIFE || weapon_idx == WEAPON_KNIFE_T || weapon_idx == knife_idx) {
            mem.write<short>(current_weapon + offs.m_iItemDefinitionIndex, knife_idx);
            mem.write<UINT>(current_weapon + offs.m_nModelIndex, model_index);
            mem.write<UINT>(current_weapon + offs.m_iViewModelIndex, model_index);
            mem.write<int>(current_weapon + offs.m_iEntityQuality, ENTITY_QUALITY);
            skin = WeaponSkin("", weapon_idx, cfg.skins_knife_skin_id, 0.0001f);
        }

        if (skin.kit != 0) { // set skin properties
            mem.write<int>(current_weapon + offs.m_iItemIDHigh, ITEM_ID_HIGH);
            mem.write<UINT>(current_weapon + offs.m_nFallbackPaintKit, skin.kit);
            mem.write<float>(current_weapon + offs.m_flFallbackWear, skin.wear);
        }
    }

    // get entity of weapon in our hands
    auto active_weapon = get_handle_entity(mem, offs, local + offs.m_hActiveWeapon);
    if (!active_weapon) {
        return;
    }

    auto weapon_idx = mem.read<short>(active_weapon + offs.m_iItemDefinitionIndex);
    if (weapon_idx != knife_idx) { // skip if current weapon is not already set to chosen knife
        return;
    }

    // get viewmodel entity
    auto view_model = get_handle_entity(mem, offs, local + offs.m_hViewModel);
    if (!view_model) {
        return;
    }

    mem.write<UINT>(view_model + offs.m_nModelIndex, model_index);
}
