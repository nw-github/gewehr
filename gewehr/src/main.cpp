#include <stdafx.h>

#include "utils/options.hpp"
#include "utils/utils.hpp"
#include "utils/offsets.hpp"
#include "utils/render.hpp"
#include "utils/memory.hpp"

#include "features/skinchanger.hpp"
#include "features/player.hpp"
#include "features/visuals.hpp"

using namespace std::chrono_literals;

static void print_menu() {
#define IFENABLED(x) x ? xorstr("enabled ") : xorstr("disabled")
#define VK2STR(x)    utl::vk_to_string(x)

    utl::clear_console();
    utl::write_line(xorstr("\tGewehr (Built on {} at {})\n"), xorstr(__DATE__), xorstr(__TIME__));
    utl::write_line(xorstr("close:          {}"), VK2STR(g::options->exit_key));
    utl::write_line(xorstr("refresh config: {}\n"), VK2STR(g::options->refresh_cfg_key));
    utl::write_line(xorstr("skinchanger:    {}"), IFENABLED(g::options->skins_enabled));
    utl::write_line(xorstr("esp:            {} [{}]"), IFENABLED(g::options->esp_enabled), VK2STR(g::options->esp_toggle_key));
    utl::write_line(xorstr("bhop:           {} [{}]"), IFENABLED(g::options->bhop_enabled), VK2STR(g::options->bhop_toggle_key));
    utl::write_line(xorstr("visuals:        {} [{}] (glow {}, chams {})"),
        IFENABLED(g::options->visuals_enabled), VK2STR(g::options->visuals_toggle_key),
        IFENABLED(g::options->glow_enabled), IFENABLED(g::options->chams_enabled));
    utl::write_line(xorstr("rcs:            {} [{}] (X: {:.2}, Y: {:.2}, after {} shots)"), IFENABLED(g::options->rcs_enabled),
        VK2STR(g::options->rcs_toggle_key), g::options->rcs_strength_x, g::options->rcs_strength_y, g::options->rcs_after_shots);
    utl::write_line(xorstr("triggerbot:     {} [{}] ({} ms, hold {})"), IFENABLED(g::options->trigger_enabled),
        VK2STR(g::options->trigger_toggle_key), g::options->trigger_delay, VK2STR(g::options->trigger_key));

#undef IFENABLED
#undef VK2STR
}

class GameManager {
public:
    enum Feature {
        RENDERER,
        SKINCHANGER,
        BHOP,
        TBOT,
        RCS,
        VISUALS,
        MAX,
    };

public:
    ~GameManager() {
        join_threads();
    }

    bool attach_to_game() {
        utl::clear_console();
        utl::write_line(xorstr("[!] Waiting for process..."));

        while (!g::memory->attach(xorstr("csgo.exe"), xorstr("Counter-Strike: Global Offensive - Direct3D 9")))
            std::this_thread::sleep_for(2000ms);
        while (!g::memory->find_modules())
            std::this_thread::sleep_for(2000ms);

        utl::write_line(xorstr("[+] Game window: {}"), fmt::ptr(g::memory->window));
        utl::write_line(xorstr("[+] Attached to PID {}: {}"),
            g::memory->process_id, fmt::ptr(g::memory->process.get()));
        utl::write_line(xorstr("[+] Found module \"engine.dll\": {:#x}"),
            g::memory->engine_dll.get_image_base());
        utl::write_line(xorstr("[+] Found module \"client.dll\": {:#x}"),
            g::memory->client_dll.get_image_base());

        if (!g::offsets->initialize()) {
            utl::write_line(xorstr("[-] Failed to obtain offsets!"));
            return false;
        }

        if (g::config->exists()) {
            g::config->load_all();
        } else {
            g::config->save_all();
        }
        return true;
    }

    void detach_from_game() {
        utl::write_line("");
        Beep(900, 500);

        join_threads();
        g::memory->detach();
    }

    void join_threads() {
        for (auto &thread : m_threads) {
            if (thread.joinable()) {
                thread.request_stop();
                thread.join();
            }
        }
    }

    void toggle_feature(Feature feature, bool option) {
#define START_THREAD(fn) m_threads[feature] = std::jthread([] (std::stop_token token) { (fn)(token); })
        if (option) {
            switch (feature) {
            case Feature::RENDERER:
                START_THREAD(g::render->thread_proc);
                break;
            case Feature::VISUALS:
                START_THREAD(f::visuals->thread_proc);
                break;
            case Feature::BHOP:
                START_THREAD(f::player->bhop_thread_proc);
                break;
            case Feature::TBOT:
                START_THREAD(f::player->tbot_thread_proc);
                break;
            case Feature::RCS:
                START_THREAD(f::player->rcs_thread_proc);
                break;
            case Feature::SKINCHANGER:
                START_THREAD(f::skins->thread_proc);
                break;
            }
        } else if (m_threads[feature].joinable()) {
            m_threads[feature].request_stop();
            m_threads[feature].join();
        }
#undef START_THREAD
    }

private:
    std::array<std::jthread, Feature::MAX> m_threads{};

};

static DWORD WINAPI main_thread_proc(void *param) {
    static GameManager manager;

    utl::attach_console();
    if (manager.attach_to_game()) {
        std::this_thread::sleep_for(1000ms);
        print_menu();

        manager.toggle_feature(GameManager::RENDERER, g::options->esp_enabled);
        manager.toggle_feature(GameManager::VISUALS, g::options->visuals_enabled);
        manager.toggle_feature(GameManager::BHOP, g::options->bhop_enabled);
        manager.toggle_feature(GameManager::RCS, g::options->rcs_enabled);
        manager.toggle_feature(GameManager::TBOT, g::options->trigger_enabled);
        manager.toggle_feature(GameManager::SKINCHANGER, g::options->skins_enabled);

        while (!utl::is_key_down(g::options->exit_key)) {
#define TOGGLE_KEY(feature, key, enabled) \
    do { \
        if (utl::is_key_down(key)) {\
            manager.toggle_feature(feature, (enabled) = !(enabled)); \
            Beep((enabled) ? 1000 : 900, 250); \
            print_menu(); \
        } \
 } while (false)
            
            TOGGLE_KEY(GameManager::VISUALS, g::options->visuals_toggle_key, g::options->visuals_enabled);
            TOGGLE_KEY(GameManager::BHOP, g::options->bhop_toggle_key, g::options->bhop_enabled);
            TOGGLE_KEY(GameManager::RCS, g::options->rcs_toggle_key, g::options->rcs_enabled);
            TOGGLE_KEY(GameManager::TBOT, g::options->trigger_toggle_key, g::options->trigger_enabled);
            TOGGLE_KEY(GameManager::RENDERER, g::options->esp_toggle_key, g::options->esp_enabled);
            
#undef TOGGLE_KEY

            if (utl::is_key_down(g::options->refresh_cfg_key)) {
                Beep(900, 400);
                g::config->load_all();
                print_menu();
            }

            DWORD code = 0;
            if (GetExitCodeProcess(g::memory->process.get(), &code)) {
                if (code != STILL_ACTIVE) {
                    manager.detach_from_game();
                    return main_thread_proc(param);
                }
            }

            std::this_thread::sleep_for(500ms);
        }
    }

    manager.detach_from_game();
    FreeLibraryAndExitThread(static_cast<HINSTANCE>(param), 0);
    return 0;
}

static BOOL WINAPI dll_exit() {
    g::memory->detach();
    utl::detach_console();
#ifdef _DEBUG
    ExitProcess(0);
#endif
    return TRUE;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved) {
    switch (dwReason) {
    case DLL_PROCESS_ATTACH: {
        DisableThreadLibraryCalls(hModule);

        if (HANDLE thread = CreateThread(nullptr, 0, main_thread_proc, hModule, 0, nullptr))
            CloseHandle(thread);

        return TRUE;
    }
    case DLL_PROCESS_DETACH: {
        if (lpReserved == nullptr)
            return dll_exit();
    } break;
    }
    return TRUE;
}