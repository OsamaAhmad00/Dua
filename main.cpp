#include <ModuleCompiler.h>

int main()
{
    ModuleCompiler module(
        "MainModule", ""
        "int main() {"
        "    int i = 4;"
        "    int j = 1;"
        "    if (i <= 4) {"
        "        j = -5;"
        "        if (!(j > 10)) {"
        "            i = 30;"
        "        } else"
        "            i = 3;"
        "    }"
        "    return ~(i * j);"
        "}"
    );
}