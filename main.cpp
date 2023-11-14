#include <ModuleCompiler.h>

int main()
{
    ModuleCompiler module(
        "MainModule", ""
        "int x() = 5;"
        ""
        "int y(int i, int j) { return i; }"
        ""
        "int main() {"
        "   int i = x();"
        "   return y(1, 2);"
        "}"
    );
}