#include <stdafx.h>

#include "config.hpp"

namespace fs = std::filesystem;

namespace {
    std::optional<fs::path> get_config_location() {
        char documents[MAX_PATH];
        if (FAILED(SHGetFolderPathA(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, documents))) {
            return std::nullopt;
        }

        return fs::path{documents} / FOLDER_NAME / xorstr("config.json");
    }
} // namespace

std::optional<Config> Config::load() {
    if (const auto path = get_config_location()) {
        nlohmann::json js;
        try {
            if (std::ifstream file{path.value()}) {
                file >> js;
            }

            return js.get<Config>();
        } catch (const std::exception &ex) {
            utl::println("Error loading config file: {}", ex.what());
        }
    }

    return std::nullopt;
}

bool Config::save() const {
    const auto path = get_config_location();
    if (!path || !fs::create_directories(path->parent_path())) {
        return false;
    }

    try {
        if (std::ofstream file{*path}) {
            file << std::setw(4) << nlohmann::json{*this};
        }
        return true;
    } catch (const std::exception &ex) {
        utl::println("Error saving config file: {}", ex.what());
    }
    return false;
}
