#pragma once

#pragma warning(push, 0)
#include <nlohmann/json.hpp>
#pragma warning(pop)

#include "../features/skinchanger.hpp"
#include "../utils/render.hpp"
#include "../utils/utils.hpp"

#define FOLDER_NAME xorstr(".gw")

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(WeaponSkin, name, id, kit, wear);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Color, r, g, b, a);

class IOption {
public:
    virtual void save(nlohmann::json& js) const = 0;
    virtual void load(const nlohmann::json& js) = 0;
};

template<typename T>
class Option final : public IOption {
public:
    Option(std::string_view name, T &&value)
        : name(name), value(std::forward<T>(value))
    { }

    void save(nlohmann::json &js) const override {
        js[name] = value;
    }

    void load(const nlohmann::json &js) override {
        try {
            if (js.count(name))
                value = js[name].get<T>();
        } catch (const std::exception &ex) {
            utl::write_line("Error loading option '{}': {}", name.c_str(), ex.what());
            return;
        }
    }

public:
    T value;

private:
    std::string name;

};

class Config {
public:
    Config();

    void load_all();
    void save_all() const;
    bool exists() const;

    template<typename T>
    T &add_option(Option<T> *opt) {
        m_options.emplace_back(opt);
        return opt->value;
    }

public:
    std::filesystem::path m_folderpath;
    std::filesystem::path m_configpath;
    
    mutable std::mutex m_load_mutex;

private:
    std::vector<std::unique_ptr<IOption>> m_options;

};

namespace g
{
    inline std::unique_ptr<::Config> config = std::make_unique<::Config>();
}