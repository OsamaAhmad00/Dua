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
        "    int x = 223;"
        "    int* p = &x;"
        "    int** pp = &p;"
        "    print(***&pp);"
        "    ***&pp = 56;"
        "    print(x);"
        "    return 0;"
        "}"
    );
}