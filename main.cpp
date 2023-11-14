#include <ModuleCompiler.h>

int main()
{
    ModuleCompiler module(
        "MainModule", ""
        "int main() {"
        "   if (1) 2; else if (3) 4; else 5;"
        "   if (1) 2 else if (3) 4 else 5;"
        "   when { 1 -> 2, 3 -> 4, else -> 5 };"
        "}"
    );
}