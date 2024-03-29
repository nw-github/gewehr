#include <stdafx.h>

#include "state.hpp"

std::optional<State> State::init() {
    utl::clear_console();
    utl::println(xorstr("[!] Waiting for process..."));
    std::optional<Memory> mem;
    do {
        mem = Memory::init(xorstr("csgo.exe"), xorstr("Counter-Strike: Global Offensive - Direct3D 9"));
    } while (!mem);

    utl::println(xorstr("[+] Game window: {}"), fmt::ptr(mem->window));
    utl::println(xorstr("[+] Attached to PID {}: {}"), mem->process_id, fmt::ptr(mem->process.get()));
    utl::println(xorstr("[+] Found module \"engine.dll\": {:#x}"), mem->engine_dll.base_addr);
    utl::println(xorstr("[+] Found module \"client.dll\": {:#x}"), mem->client_dll.base_addr);

    const auto offsets = Offsets::init(mem.value());
    if (!offsets) {
        utl::println(xorstr("[-] Failed to obtain offsets!"));
        return std::nullopt;
    }

    auto config = Config::load();
    if (!config) {
        config = Config{};
        config->save();
    }

    return State{
        .mem = std::move(mem.value()),
        .offsets = offsets.value(),
        .cfg = *config,
    };
}

bool State::reload_config() {
    if (const auto config = Config::load()) {
        cfg = config.value();
        return true;
    }
    return false;
}
