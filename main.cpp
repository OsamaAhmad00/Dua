#include <ModuleCompiler.h>

int main()
{
    ModuleCompiler module(
        "MainModule", ""
        "int main() {"
        "   if (1) 2 else if (3) 4 else 5;"
        "}"
    );
}