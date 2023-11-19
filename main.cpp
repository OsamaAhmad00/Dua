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
        "    print(0x0011);"
        "    print(0xFF);"
        "    print(0xFFT);"
        "    print(0b0010101);"
        "    print(0344);"
        "    print(123);"
        "    print(0x0AbCd);"
        "    print(00011T);"
        "    print(0b01010101);"
        "    print(0xFFFFFFFS);"
        "}"
    );
}