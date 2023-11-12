#include <ModuleCompiler.h>

int main()
{
    ModuleCompiler module(
        "MainModule", ""
        "int main() {"
        "   if (3) {"
        "       int x = 3;"
        "   } else if (4)"
        "       5;"
        "   else if (5) {"
        "       int y = 3;"
        "   } else {"
        "       6;"
        "   }"
        "}"
    );
}