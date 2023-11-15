#include <ModuleCompiler.h>

int main()
{
    ModuleCompiler module(
        "MainModule", ""
        "int printf(i8* message, ...);"
        ""
        "int print(int num) = { printf(\"%d\n\", num); 0 }"
        ""
        "int main() {"
        "    print(3 ^ 5);"
        "    print(22 << 3);"
        "    print(4390 >> 5);"
        "    print(112 >>> 5);"
        "    print(122 & 23);"
        "    print(432 | 23);"
        "    return 0;"
        "}"
    );
}