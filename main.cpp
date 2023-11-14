#include <ModuleCompiler.h>

int main()
{
    ModuleCompiler module(
        "MainModule", ""
        "int main() {"
        "   int i = { int j = 3; j };"
        "   return i;"
        "}"
    );
}