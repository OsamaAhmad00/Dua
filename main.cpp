#include <ModuleCompiler.h>

int main()
{
    dua::ModuleCompiler module(
        "MainModule", ""
        "int printf(i8* message, ...);"
        ""
        "void print(i32 num) { printf(\"%d\n\", num); }"
        ""
        "int main() {"
        "    if (0 && 1 < 3 && 5 && *(int*)0) print(5);"
        "    else if (1 || *(int*)0) print(6);"
        "    return 0;"
        "}"
    );
}