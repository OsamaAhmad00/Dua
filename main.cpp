#include <ModuleCompiler.h>

int main()
{
    ModuleCompiler module(
        "MainModule", ""
        "int x() = 6;"
        ""
        "int y(int i, int j) { return i * j; }"
        ""
        "int main() {"
        "   return x() + y(4, 2);"
        "}"
    );
}