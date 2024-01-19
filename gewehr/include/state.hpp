#pragma once
#include "config.hpp"
#include "memory.hpp"
#include "offsets.hpp"

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
