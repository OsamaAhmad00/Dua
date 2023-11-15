#include <ModuleCompiler.h>

int main()
{
    ModuleCompiler module(
        "MainModule", ""
        "int printf(i8* message, ...);"
        ""
        "int main() {"
        "    int x = 3;"
        "    int* p = &x;"
        "    *p = 5;"
        "    printf(\"%d\n\", x);"
        "    return 0;"
        "}"
    );
}