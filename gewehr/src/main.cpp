#include <stdafx.h>

#include "state.hpp"
#include "features.hpp"
#include "utils.hpp"

using namespace std::chrono_literals;

namespace {
void print_menu(const Config &cfg);
}

struct Feature
{
    using ThreadProc = std::function<void(std::stop_token, const State &)>;

public:
    Feature(const State &s, ThreadProc proc, SHORT &toggle_key, bool &enabled)
        : thread_proc(proc), thr(std::nullopt), toggle_key(toggle_key), enabled(enabled)
    {
        if (enabled)
        {
            this->start(s);
        }
    }

    ~Feature() {
        stop();
    }

    void start(const State &s) {
        if (!thr) {
            thr = std::jthread(thread_proc, std::ref(s));
        }
    }

    void stop() {
        if (thr.has_value()) {
            thr->request_stop();
            thr->join();
            thr = std::nullopt;
        }
    }

    void enable(const State &s) {
        if (enabled) {
            start(s);
        } else {
            stop();
        }
    }

    void check_key(const State &s) {
        if (utl::is_key_down(toggle_key)) {
            enabled = !enabled;
            enable(s);
            Beep((enabled) ? 1000 : 900, 250);
            print_menu(s.cfg);
        }
    }

private:
    ThreadProc thread_proc;
    std::optional<std::jthread> thr;
    bool &enabled;
    SHORT &toggle_key;

};

namespace {
    void print_menu(const Config &cfg) {
#define IFENABLED(x) x ? xorstr("enabled ") : xorstr("disabled")
#define VK2STR(x) utl::vk_to_string(x)

        utl::clear_console();
        utl::println(xorstr("\tGewehr (Built on {} at {})\n"), xorstr(__DATE__), xorstr(__TIME__));
        utl::println(xorstr("close:          {}"), VK2STR(cfg.exit_key));
        utl::println(xorstr("refresh config: {}\n"), VK2STR(cfg.refresh_cfg_key));
        utl::println(xorstr("skinchanger:    {}"), IFENABLED(cfg.skins_enabled));
        utl::println(xorstr("bhop:           {} [{}]"), IFENABLED(cfg.bhop_enabled), VK2STR(cfg.bhop_toggle_key));
        utl::println(xorstr("visuals:        {} [{}] (glow {}, chams {})"),
                     IFENABLED(cfg.visuals_enabled), VK2STR(cfg.visuals_toggle_key),
                     IFENABLED(cfg.glow_enabled), IFENABLED(cfg.chams_enabled));
        utl::println(xorstr("rcs:            {} [{}] (X: {:.2}, Y: {:.2}, after {} shots)"), IFENABLED(cfg.rcs_enabled),
                     VK2STR(cfg.rcs_toggle_key), cfg.rcs_strength_x, cfg.rcs_strength_y, cfg.rcs_after_shots);
        utl::println(xorstr("triggerbot:     {} [{}] ({} ms, hold {})"), IFENABLED(cfg.trigger_enabled),
                     VK2STR(cfg.trigger_toggle_key), cfg.trigger_delay, VK2STR(cfg.trigger_key));

#undef IFENABLED
#undef VK2STR
    }

    bool execute() {
        auto _s = State::init();
        if (!_s) {
            return false;
        }
        auto &s = _s.value();
        auto &cfg = s.cfg;
        std::this_thread::sleep_for(1000ms);
        print_menu(cfg);

        std::array<Feature, 4> features{
            Feature(s, visuals::thread_proc, cfg.visuals_toggle_key, cfg.visuals_enabled),
            Feature(s, player::bhop_thread_proc, cfg.bhop_toggle_key, cfg.bhop_enabled),
            Feature(s, player::rcs_thread_proc, cfg.rcs_toggle_key, cfg.rcs_enabled),
            Feature(s, player::tbot_thread_proc, cfg.trigger_toggle_key, cfg.trigger_enabled),
        };

        while (!utl::is_key_down(cfg.exit_key)) {
            for (auto &feature : features) {
                feature.check_key(s);
            }

            if (utl::is_key_down(cfg.refresh_cfg_key)) {
                Beep(900, 400);
                if (s.reload_config()) {
                    for (auto &feature : features) {
                        feature.enable(s);
                    }
                }
                print_menu(s.cfg);
            }

            DWORD code = 0;
            if (GetExitCodeProcess(s.mem.process.get(), &code)) {
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
        if (lpReserved == nullptr) {
            utl::detach_console();
    #ifdef _DEBUG
            ExitProcess(0);
    #endif
        }
        return TRUE;
    }
    }
    return TRUE;
}
