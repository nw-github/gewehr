#pragma once

#include "../utils/entity.hpp"

class Visuals {
public:
    void thread_proc(std::stop_token token);

private:
    void apply_glow(BasePlayer &player);
    void apply_chams(BasePlayer &player);
    void render_esp(BasePlayer &player);

    void set_chams_brightness();

private:
    LocalPlayer m_local;
    matrix4x4_t m_wts_matrix{0};
    DWORD       m_dwGlowObjManager{0};

};

namespace f {
    inline std::unique_ptr<::Visuals> visuals = std::make_unique<::Visuals>();
}