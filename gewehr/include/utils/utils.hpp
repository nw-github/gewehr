#pragma once
#include "memory.hpp"

namespace utl {
    void attach_console();
    void detach_console();
    void clear_console();

    std::string vk_to_string(int vk);
    std::string randstr(int length);
    // random number from low to high, inclusive on both ends
    int randint(int low, int high);

#ifndef _DEBUG
    template<typename... Args>
    void println(std::string_view fmt, Args &&...args) {
        std::cout << fmt::vformat(fmt, fmt::make_format_args(std::forward<Args>(args)...)) << '\n';
    }
#else
    template<typename... Args>
    void println(fmt::format_string<Args...> fmt, Args &&...args) {
        std::cout << fmt::format(fmt, std::forward<Args>(args)...) << '\n';
    }
#endif

    inline bool is_key_down(int key) {
        return GetAsyncKeyState(key);
    }

    inline bool icompare(std::string_view left, std::string_view right) {
        if (left.length() != right.length()) {
            return false;
        }

        return _stricmp(left.data(), right.data()) == 0;
    }

}