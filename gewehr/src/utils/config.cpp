#include <stdafx.h>
#include <shlobj.h>

#include "utils/config.hpp"

Config::Config() {
    char documents[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, documents))) {
        m_folderpath = std::filesystem::path{documents} / FOLDER_NAME;
        m_configpath = m_folderpath / xorstr("config.json");
    }
}

void Config::save_all() const {
    if (!std::filesystem::exists(m_folderpath))
        std::filesystem::create_directory(m_folderpath);

    nlohmann::json js;
    for (const auto& ptr : m_options)
        ptr->save(js);

    std::ofstream file{m_configpath};
    if (file)
        file << std::setw(4) << js;
}

void Config::load_all() {
    nlohmann::json js;
    try {
        if (std::ifstream file{m_configpath})
            file >> js;
    } catch (const std::exception &ex) {
        utl::write_line("Error loading config file: {}", ex.what());
        return;
    }

    std::scoped_lock lock{m_load_mutex};
    for (auto &ptr : m_options)
        ptr->load(js);
}

bool Config::exists() const {
    return std::filesystem::exists(m_configpath);
}