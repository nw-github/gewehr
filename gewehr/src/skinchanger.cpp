#include <stdafx.h>

#include "features.hpp"

#include "utils/memory.hpp"
#include "utils/offsets.hpp"

// credits: xSkins [https://github.com/0xf1a/xSkins]

using namespace std::chrono_literals;

namespace {
    constexpr short KNIFE_IDS[19] = {
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

    UINT get_model_idx_by_name(const Game &game, const char* name) {
        const auto &[mem, offsets, _] = game;

        DWORD cstate = mem.read<DWORD>(offsets.dwClientState);
        DWORD nst    = mem.read<DWORD>(cstate + offsets.m_dwModelPrecache); // CClientState + 0x529C -> INetworkStringTable* m_pModelPrecacheTable
        DWORD nsd    = mem.read<DWORD>(nst + 0x40);                         // INetworkStringTable + 0x40 -> INetworkStringDict* m_pItems
        DWORD nsdi   = mem.read<DWORD>(nsd + 0xC);                          // INetworkStringDict + 0xC -> void* m_pItems

        for (UINT i = 0; i < 1024; i++) {
            DWORD nsdi_i = mem.read<DWORD>(nsdi + 0xC + i * 0x34);

            char str[128] = {0};
            if (mem.read(nsdi_i, str, sizeof(str)))
                if (_stricmp(str, name) == 0)
                    return i;
        }
        return 0;
    }

    UINT get_model_idx(const Game &s, const short itemIndex) {
        switch (itemIndex) {
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

    WeaponSkin get_weapon_skin(const Options &options, const short item_idx) {
        if (options.skin_map.count(item_idx))
            return options.skin_map.at(item_idx);
        return WeaponSkin("", item_idx, 0, 0.0001f);
    }
}

void skin_changer::thread_proc(std::stop_token token, const Game &game) {
    const auto &[mem, offsets, options] = game;

    const int itemIDHigh    = -1;
    const int entityQuality = 3;

    UINT modelIndex = 0;
    DWORD localPlayer = 0;
    short last_knife_id = KNIFE_IDS[options.skins_knife_id];

    while (!token.stop_requested()) {
        if (options.skins_enabled) {
            // model index is different for each server and map
            // below is a simple way to keep track of local base in order to reset model index
            // while also avoiding doing unnecessary extra reads because of the external RPM overhead
            DWORD tempPlayer = mem.read<DWORD>(offsets.dwLocalPlayer);
            if (!tempPlayer) { // client not connected to any server (works most of the time)
                modelIndex = 0;
                std::this_thread::sleep_for(1ms);
                continue;
            } else if (tempPlayer != localPlayer) { // local base changed (new server join/demo record)
                localPlayer = tempPlayer;
                modelIndex = 0;
            }

            short knife_idx = KNIFE_IDS[options.skins_knife_id];
            if (last_knife_id != knife_idx) {
                modelIndex = 0;
                last_knife_id = knife_idx;
            }

            while (!modelIndex && !token.stop_requested())
                modelIndex = get_model_idx(game, knife_idx);

            // loop through m_hMyWeapons slots (8 will be enough)
            for (UINT i = 0; i < 8; i++) {
                // get entity of weapon in current slot
                DWORD currentWeapon = mem.read<DWORD>(localPlayer + offsets.m_hMyWeapons + i * 0x4) & 0xfff;
                currentWeapon        = mem.read<DWORD>(offsets.dwEntityList2 + (currentWeapon - 1) * 0x10);
                if (!currentWeapon) { continue; }
                
                short weaponIndex    = mem.read<short>(currentWeapon + offsets.m_iItemDefinitionIndex);
                WeaponSkin weaponSkin = get_weapon_skin(options, weaponIndex);

                // for knives, set item and model related properties
                if (weaponIndex == WEAPON_KNIFE || weaponIndex == WEAPON_KNIFE_T || weaponIndex == knife_idx) {
                    mem.write<short>(currentWeapon + offsets.m_iItemDefinitionIndex, knife_idx);
                    mem.write<UINT>(currentWeapon + offsets.m_nModelIndex            , modelIndex);
                    mem.write<UINT>(currentWeapon + offsets.m_iViewModelIndex        , modelIndex);
                    mem.write<int>(currentWeapon + offsets.m_iEntityQuality        , entityQuality);
                    weaponSkin = WeaponSkin("", weaponIndex, options.skins_knife_skin_id, 0.0001f);
                }

                if (weaponSkin.kit != 0) { // set skin properties
                    mem.write<int>(currentWeapon + offsets.m_iItemIDHigh            , itemIDHigh);
                    mem.write<UINT>(currentWeapon + offsets.m_nFallbackPaintKit    , weaponSkin.kit);
                    mem.write<float>(currentWeapon + offsets.m_flFallbackWear        , weaponSkin.wear);
                }
            }

            // get entity of weapon in our hands
            DWORD activeWeapon = mem.read<DWORD>(localPlayer + offsets.m_hActiveWeapon) & 0xfff;
            activeWeapon = mem.read<DWORD>(offsets.dwEntityList2 + (activeWeapon - 1) * 0x10);
            if (!activeWeapon) 
                continue;
            
            short weaponIndex = mem.read<short>(activeWeapon + offsets.m_iItemDefinitionIndex);
            if (weaponIndex != knife_idx) // skip if current weapon is not already set to chosen knife
                continue;

            // get viewmodel entity
            DWORD activeViewModel = mem.read<DWORD>(localPlayer + offsets.m_hViewModel) & 0xfff;
            activeViewModel = mem.read<DWORD>(offsets.dwEntityList2 + (activeViewModel - 1) * 0x10);
            if (!activeViewModel)
                continue;

            mem.write<UINT>(activeViewModel + offsets.m_nModelIndex, modelIndex);
        } else {
            std::this_thread::sleep_for(1ms);
        }
    }
}
