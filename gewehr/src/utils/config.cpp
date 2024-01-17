#include <stdafx.h>
#include <shlobj.h>

#include "utils/config.hpp"

namespace fs = std::filesystem;

// TODO: unglobalify
namespace {
    fs::path folder;
    fs::path config;
}

std::optional<Options> Options::load() {
    char documents[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, documents))) {
        folder = fs::path{documents} / FOLDER_NAME;
        config = folder / xorstr("config.json");

        nlohmann::json js;
        try {
            if (std::ifstream file{config})
                file >> js;

            return js.get<Options>();
        } catch (const std::exception &ex) {
            utl::println("Error loading config file: {}", ex.what());
        }
    }

    return std::nullopt;
}

bool Options::save() const {
    if (!std::filesystem::exists(folder) || !std::filesystem::create_directory(folder))
        return false;

    try {
        std::ofstream file{config};
        if (file)
            file << std::setw(4) << nlohmann::json{*this};
        return true;
    } catch (const std::exception &) {
        return false;
    }
}
