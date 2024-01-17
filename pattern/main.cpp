#include <string>
#include <iostream>

static inline void replace(std::string& str, const std::string& from, const std::string& to)
{
    size_t pos = 0;
    while ((pos = str.find(from, pos)) != std::string::npos)
    {
        str.replace(pos, from.length(), to);
        pos += to.length();
    }
}

static inline std::string pattern_convert(std::string pattern, const std::string& wildcard)
{
    replace(pattern, " ", "\\x");
    replace(pattern, "?", wildcard);

    return "\\x" + pattern;
}

int main(int argc, char** argv)
{
    while (true) {
        std::cout << "enter pattern: ";

        std::string pattern;
        std::getline(std::cin, pattern);

        std::cout << pattern_convert(pattern, "AA") << std::endl << std::endl;
    }

    return 0;
}