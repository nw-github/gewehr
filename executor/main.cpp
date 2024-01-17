#include <windows.h>
#include <string>
#include <iostream>

int main(int argc, char** argv)
{
    char buffer[MAX_PATH];
    GetFullPathName("gewehr.dll", MAX_PATH, buffer, nullptr);
    HMODULE hModule = LoadLibraryA(buffer);

    while (true)
        Sleep(1000);
}