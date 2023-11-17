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
        "    int[10] arr;"
        "    for (int i = 1; i <= 10; i++)"
        "        arr[i - 1] = i;"
        "    for (int i = 9; i >= 0; i--)"
        "        print(arr[i]);"
        "    return 0;"
        "}"
    );
}