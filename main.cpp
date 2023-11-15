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
        "    int** pp = &p;"
        "    int*** ppp = &pp;"
        "    ***ppp = 5 % 3;"
        "    printf(\"%d\n\", x);"
        "    return 0;"
        "}"
    );
}