#pragma once
#include "game.hpp"

struct SkinChanger {
public:
    SkinChanger(const Options& options);

    void tick(const std::stop_token &token, const Game &game);

private:
    UINT modelIndex;
    DWORD localPlayer;
    short last_knife_id;
    
};

namespace player {
    void bhop_thread_proc(std::stop_token token, const Game &game);
    void tbot_thread_proc(std::stop_token token, const Game &game);
    void rcs_thread_proc(std::stop_token token, const Game &game);
}

namespace visuals {
    void thread_proc(std::stop_token token, const Game &game);
}
