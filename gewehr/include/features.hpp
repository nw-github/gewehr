#pragma once
#include "game.hpp"

namespace player {
    void bhop_thread_proc(std::stop_token token, const Game &game);
    void tbot_thread_proc(std::stop_token token, const Game &game);
    void rcs_thread_proc(std::stop_token token, const Game &game);
}

namespace visuals {
    void thread_proc(std::stop_token token, const Game &game);
}

namespace skin_changer {
    void thread_proc(std::stop_token token, const Game &game);
}
