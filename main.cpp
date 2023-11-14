#include <ModuleCompiler.h>

int main()
{
    ModuleCompiler module(
        "MainModule", ""
        "int main() {"
        "    if (4 < 3 == true) return 5;"
        "    else if (5 < 3 == false) return 6;"
        "    else return 1;"
        "}"
    );
}