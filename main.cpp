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
        "    int c = 0;"
        "    for (int i = 0, int j = 5, short k = 5; i <= 5; { i++; j-- })"
        "        c += i * j;"
        "    print(c);"
        "    for(;;);"
        "}"
    );
}