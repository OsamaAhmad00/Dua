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
        "    when {"
        "        1 -> 2,"
        "        3 -> 4,"
        "        5 -> 6,"
        "        else -> 7"
        "    };"
        "    if (1) 1 else if (2) 2 else 3;"
        "    return 0;"
        "}"
    );
}