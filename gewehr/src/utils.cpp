#include <stdafx.h>

#include "utils.hpp"

#include <random>

namespace {
    thread_local std::mt19937 rng{std::random_device{}()};
}

void utl::attach_console() {
    AllocConsole();
    AttachConsole(GetCurrentProcessId());

    (void)freopen(xorstr("CONOUT$"), "w", stdout);
    HANDLE new_out = CreateFileA(xorstr("CONOUT$"),
                                 GENERIC_READ | GENERIC_WRITE,
                                 FILE_SHARE_READ | FILE_SHARE_WRITE,
                                 NULL,
                                 OPEN_EXISTING,
                                 FILE_ATTRIBUTE_NORMAL,
                                 NULL);

    SetStdHandle(STD_OUTPUT_HANDLE, new_out);
    SetConsoleTitleA(utl::randstr(utl::randint(10, 15)).c_str());

    CONSOLE_CURSOR_INFO cursor_info = {0};
    cursor_info.dwSize              = sizeof(cursor_info);
    cursor_info.bVisible            = false;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor_info);
}

void utl::detach_console() {
    FreeConsole();
}

void utl::clear_console() {
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    if (out == INVALID_HANDLE_VALUE)
        return;

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (!GetConsoleScreenBufferInfo(out, &csbi))
        return;

    COORD home  = {0, 0};
    DWORD cells = csbi.dwSize.X * csbi.dwSize.Y;
    DWORD count;
    if (!FillConsoleOutputCharacterA(out, ' ', cells, home, &count))
        return;
    if (!FillConsoleOutputAttribute(out, csbi.wAttributes, cells, home, &count))
        return;

    SetConsoleCursorPosition(out, home);
}

std::string utl::vk_to_string(int vk) {
    std::string buf(10, '\0');

    UINT sc = MapVirtualKeyA(vk, MAPVK_VK_TO_VSC_EX);
    switch (vk) {
    case VK_LEFT:
    case VK_UP:
    case VK_RIGHT:
    case VK_DOWN:
    case VK_RCONTROL:
    case VK_RMENU:
    case VK_LWIN:
    case VK_RWIN:
    case VK_APPS:
    case VK_PRIOR:
    case VK_NEXT:
    case VK_END:
    case VK_HOME:
    case VK_INSERT:
    case VK_DELETE:
    case VK_DIVIDE:
    case VK_NUMLOCK:
        sc |= KF_EXTENDED;
        break;
    };

    int length = GetKeyNameTextA(sc << 16, buf.data(), buf.size());
    if (length <= 0)
        return xorstr("none");

    buf.resize(length);
    return buf;
}

int utl::randint(int low, int high) {
    return std::uniform_int_distribution{low, high}(rng);
}

std::string utl::randstr(int length) {
    static const std::string chars = xorstr("0123456789"
                                            "!@#$%^&*"
                                            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                            "abcdefghijklmnopqrstuvwxyz");

    std::string str(length, '\0');
    std::generate(str.begin(), str.end(), [] {
        return chars[utl::randint(0, chars.length() - 1)];
    });
    return str;
}
