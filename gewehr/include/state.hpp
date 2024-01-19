#pragma once
#include "utils/config.hpp"
#include "utils/memory.hpp"
#include "utils/offsets.hpp"

class State {
public:
    static std::optional<State> init();
    bool reload_config();

    bool has_focus() const {
        return GetForegroundWindow() == mem.window;
    }

public:
    Memory mem;
    Offsets offsets;
    // todo: race conditions
    Config cfg;

};
