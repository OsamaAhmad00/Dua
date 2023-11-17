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
        "    printf(\"Hello, world!\n\");"
        "    printf(\"Hello, world!\n\");"
        "    printf(\"Helloooo, world!\n\");"
        "    printf(\"Hello, world!\n\");"
        "    printf(\"Hello, world!\n\");"
        "    return 0;"
        "}"
    );
}