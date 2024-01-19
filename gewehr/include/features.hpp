#pragma once
#include "state.hpp"

struct SkinChanger {
public:
    SkinChanger(const Config &cfg);

    void tick(const std::stop_token &token, const State &s);

private:
    UINT model_index;
    DWORD local;
    short last_knife_id;
};

namespace player {
    void bhop_thread_proc(std::stop_token token, const State &s);
    void tbot_thread_proc(std::stop_token token, const State &s);
    void rcs_thread_proc(std::stop_token token, const State &s);
} // namespace player

namespace visuals {
    void thread_proc(std::stop_token token, const State &s);
}
