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
        "    int x = 5;"
        "    do print(x) while (x < 3);"
        "    do {"
        "        print(x);"
        "        x--;"
        "    } while (x > 0);"
        "    return 0;"
        "}"
    );
}