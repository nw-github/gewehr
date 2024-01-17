#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN

#pragma warning(push, 0)
#include <windows.h>
#include <tlhelp32.h>

#include <wil/resource.h>

#include <io.h>
#include <fcntl.h>

#include <vector>
#include <string>
#include <queue>
#include <thread>
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <mutex>
#include <memory>
#include <variant>
#include <optional>

#include <cstdarg>
#include <cstdio>

#include <valve_sdk/sdk.hpp>

#define FMT_HEADER_ONLY
#include <fmt/format.h>

#include "xorstr.hpp"
#pragma warning(pop)