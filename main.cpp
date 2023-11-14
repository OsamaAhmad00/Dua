#include <ModuleCompiler.h>

int main()
{
    ModuleCompiler module(
        "MainModule", ""
        "int* main() {"
        "    int i = 3;"
        "    return &i;"
        "}"
    );
}