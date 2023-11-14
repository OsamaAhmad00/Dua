#include <ModuleCompiler.h>

int main()
{
    ModuleCompiler module(
        "MainModule", ""
        "int main() {"
        "   int i = 0;"
        "   int j;"
        "   j = 3;"
        "   return 5;"
        "}"
    );
}