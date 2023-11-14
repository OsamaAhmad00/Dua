#include <ModuleCompiler.h>

int main()
{
    ModuleCompiler module(
        "MainModule", ""
        "int printf(i8* message, ...);"
        ""
        "int main() {"
        "    i8* str = \"Hello, world!\";"
        "    printf(str);"
        "    return 0;"
        "}"
    );
}