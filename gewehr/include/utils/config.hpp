#pragma once

#pragma warning(push, 0)
#include <nlohmann/json.hpp>
#pragma warning(pop)

#include "../utils/render.hpp"
#include "../utils/utils.hpp"

#define FOLDER_NAME xorstr(".gw")

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
            utl::println("Error loading option '{}': {}", name.c_str(), ex.what());
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

struct WeaponSkin {
    std::string name;
    SHORT       id{0};
    UINT        kit{0};
    float       wear{0.0001f};

public:
    WeaponSkin() = default;

    WeaponSkin(const char* name, SHORT weapon_id, UINT paint_kit, float fallback_wear)
        : name{ name }, id{ weapon_id }
        , kit{ paint_kit }, wear{ fallback_wear }
    { }

};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(WeaponSkin, name, id, kit, wear);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Color, r, g, b, a);

namespace g
{
    inline std::unique_ptr<::Config> config = std::make_unique<::Config>();
}