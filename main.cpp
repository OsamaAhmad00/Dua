#include <ModuleCompiler.h>

int main()
{
    ModuleCompiler module(
        "MainModule", ""
        "int printf(i8* message, ...);"
        ""
        "int main() {"
        "    int x = 3;"
        "    x++;"
        "    printf(\"%d\n\", x);"
        "    x--; --x;"
        "    printf(\"%d\n\", x++);"
        "    printf(\"%d\n\", x);"
        "    printf(\"%d\n\", --x);"
        "    return 0;"
        "}"
    );
}