#include <ModuleCompiler.h>

int main()
{
    ModuleCompiler module(
        "MainModule", ""
        "int main() {"
        "    int[5][3][6] i;"
        "    return 3;"
        "}"
    );
}