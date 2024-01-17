#include <stdafx.h>

#include "game.hpp"

std::optional<Game> Game::init() {
    utl::clear_console();
    utl::println(xorstr("[!] Waiting for process..."));
    std::optional<Memory> mem;
    do  {
        mem = Memory::init(
            xorstr("csgo.exe"),
            xorstr("Counter-Strike: Global Offensive - Direct3D 9")
        );
    } while (!mem);

    if (!mem) {
        return std::nullopt;
    }

    utl::println(xorstr("[+] Game window: {}"), fmt::ptr(mem->window));
    utl::println(
        xorstr("[+] Attached to PID {}: {}"), 
        mem->process_id, 
        fmt::ptr(mem->process.get())
    );
    utl::println(xorstr("[+] Found module \"engine.dll\": {:#x}"), mem->engine_dll.get_image_base());
    utl::println(xorstr("[+] Found module \"client.dll\": {:#x}"),
        mem->client_dll.get_image_base());

    const auto offsets = Offsets::init(mem.value());
    if (!offsets) {
        utl::println(xorstr("[-] Failed to obtain offsets!"));
        return std::nullopt;
    }

    return Game {
        .mem = std::move(mem.value()),
        .offsets = offsets.value(),
        .options = Options::load().value_or(Options{}),
    };
}

void Game::reload_config() {
    if (const auto config = Options::load()) {
        options = config.value();
    }
}