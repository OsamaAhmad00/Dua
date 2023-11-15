#include <ModuleCompiler.h>

int main()
{
    ModuleCompiler module(
        "MainModule", ""
        "int printf(i8* message, ...);"
        ""
        "int main() {"
        "    int x = 3;"
        "    printf(\"0x%X\n\", &x);"
        "    return 0;"
        "}"
    );
}