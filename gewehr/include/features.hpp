#pragma once

namespace player {
    void bhop_thread_proc(std::stop_token token);
    void tbot_thread_proc(std::stop_token token);
    void rcs_thread_proc(std::stop_token token);
}

namespace visuals {
    void thread_proc(std::stop_token token);
}

namespace skin_changer {
    void thread_proc(std::stop_token token);
}
