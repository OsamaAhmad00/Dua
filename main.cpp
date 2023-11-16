#include <ModuleCompiler.h>

int main()
{
    ModuleCompiler module(
        "MainModule", ""
        "int printf(i8* message, ...);"
        ""
        "void print(i32 num) { printf(\"%d\n\", num); }"
        ""
        "int main() {"
        "    for (int i = 0; i < 10; i++) {"
        "        if (i % 2 == 0)"
        "            continue;"
        "        print(i);"
        "        if (i == 7)"
        "            break;"
        "    }"
        "    return 0;"
        "}"
    );
}