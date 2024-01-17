#pragma once
#include "utils/config.hpp"
#include "utils/memory.hpp"
#include "utils/offsets.hpp"

class Game {
public:
    static std::optional<Game> init();
    void reload_config();

public:
    Memory mem;
    Offsets offsets;
    // todo: race conditions
    Options options;

};
