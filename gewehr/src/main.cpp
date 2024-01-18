#include <stdafx.h>

#include "game.hpp"
#include "features.hpp"
#include "utils/utils.hpp"

using namespace std::chrono_literals;

struct Feature {
    using ThreadProc = std::function<void(std::stop_token, const Game &)>;

public:
    Feature(const Game &game, ThreadProc proc, SHORT&toggle_key, bool &enabled)
        : thread_proc(proc), thr(std::nullopt), toggle_key(toggle_key), enabled(enabled)
    {
        if (enabled) {
            this->start(game);
        }
    }

    ~Feature() {
        stop();
    }

    void start(const Game &game) {
        if (!thr) {
            thr = std::jthread(thread_proc, std::ref(game));
        }
    }

    void stop() {
        if (thr.has_value()) {
            thr->request_stop();
            thr->join();
            thr = std::nullopt;
        }
    }

    void enable(const Game &game) {
        if (enabled) {
            start(game);
        } else {
            stop();
        }
    }

    void check_key(const Game& game) {
        if (utl::is_key_down(toggle_key)) {
            enabled = !enabled;
            enable(game);
            Beep((enabled) ? 1000 : 900, 250);
            print_menu(game.options);
        }
    }

private:
    ThreadProc thread_proc;
    std::optional<std::jthread> thr;
    bool& enabled;
    SHORT &toggle_key;

};

namespace {
    void print_menu(const Options &options) {
    #define IFENABLED(x) x ? xorstr("enabled ") : xorstr("disabled")
    #define VK2STR(x)    utl::vk_to_string(x)

        utl::clear_console();
        utl::println(xorstr("\tGewehr (Built on {} at {})\n"), xorstr(__DATE__), xorstr(__TIME__));
        utl::println(xorstr("close:          {}"), VK2STR(options.exit_key));
        utl::println(xorstr("refresh config: {}\n"), VK2STR(options.refresh_cfg_key));
        utl::println(xorstr("skinchanger:    {}"), IFENABLED(options.skins_enabled));
        utl::println(xorstr("bhop:           {} [{}]"), IFENABLED(options.bhop_enabled), VK2STR(options.bhop_toggle_key));
        utl::println(xorstr("visuals:        {} [{}] (glow {}, chams {})"),
            IFENABLED(options.visuals_enabled), VK2STR(options.visuals_toggle_key),
            IFENABLED(options.glow_enabled), IFENABLED(options.chams_enabled));
        utl::println(xorstr("rcs:            {} [{}] (X: {:.2}, Y: {:.2}, after {} shots)"), IFENABLED(options.rcs_enabled),
            VK2STR(options.rcs_toggle_key), options.rcs_strength_x, options.rcs_strength_y, options.rcs_after_shots);
        utl::println(xorstr("triggerbot:     {} [{}] ({} ms, hold {})"), IFENABLED(options.trigger_enabled),
            VK2STR(options.trigger_toggle_key), options.trigger_delay, VK2STR(options.trigger_key));

    #undef IFENABLED
    #undef VK2STR
    }

    bool execute() {
        auto _game = Game::init();
        if (!_game) {
            return false;
        }
        auto &game = _game.value();
        auto &options = game.options;
        std::this_thread::sleep_for(1000ms);
        print_menu(options);

        std::array<Feature, 4> features{
            Feature(game, visuals::thread_proc, options.visuals_toggle_key, options.visuals_enabled),
            Feature(game, player::bhop_thread_proc, options.bhop_toggle_key, options.bhop_enabled),
            Feature(game, player::rcs_thread_proc, options.rcs_toggle_key, options.rcs_enabled),
            Feature(game, player::tbot_thread_proc, options.trigger_toggle_key, options.trigger_enabled),
        };

        while (!utl::is_key_down(options.exit_key)) {
            for (auto &feature : features) {
                feature.check_key(game);
            }

            if (utl::is_key_down(options.refresh_cfg_key)) {
                Beep(900, 400);
                if (game.reload_config()) {
                    for (auto &feature : features) {
                        feature.enable(game);
                    }
                }
                print_menu(game.options);
            }

            DWORD code = 0;
            if (GetExitCodeProcess(game.mem.process.get(), &code)) {
                if (code != STILL_ACTIVE) {
                    return true;
                }
            }

            std::this_thread::sleep_for(500ms);
        }

        utl::println("");
        Beep(900, 500);
        return false;
    }

    DWORD WINAPI main_thread_proc(void *hModule) {
        utl::attach_console();
        while (true) {
            if (!execute()) {
                break;
            }
        }

        FreeLibraryAndExitThread(static_cast<HINSTANCE>(hModule), 0);
        return 0;
    }

    BOOL WINAPI dll_exit() {
        utl::detach_console();
    #ifdef _DEBUG
        ExitProcess(0);
    #endif
        return TRUE;
    }
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
