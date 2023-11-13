#include <ModuleCompiler.h>

int main()
{
    ModuleCompiler module(
        "MainModule", ""
        "int main() {"
        "    while () { 3; }"
        "    return 0;"
        "}"
    );
}