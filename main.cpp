#include <ModuleCompiler.h>

int main()
{
    ModuleCompiler module(
        "MainModule", ""
        "int printf(i8* message, ...);"
        ""
        "int main() {"
        "    int x = 3;"
        "    printf(\"%d\n\", ((x < 3) ? 5 : 10));"
        "    return 0;"
        "}"
    );
}