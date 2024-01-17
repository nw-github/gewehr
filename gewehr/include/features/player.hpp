#pragma once

class Player {
public:
    void bhop_thread_proc(std::stop_token token);
    void tbot_thread_proc(std::stop_token token);
    void rcs_thread_proc(std::stop_token token);
    
};

namespace f {
    inline std::unique_ptr<::Player> player = std::make_unique<::Player>();
}