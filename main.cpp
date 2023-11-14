#include <ModuleCompiler.h>

int main()
{
    ModuleCompiler module(
        "MainModule", ""
        "int main() {"
        "   int i = 5;"
        "   int j;"
        "   j = i;"
        "   return j;"
        "}"
    );
}