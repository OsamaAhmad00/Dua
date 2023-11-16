#include <ModuleCompiler.h>

int main()
{
    ModuleCompiler module(
        "MainModule", ""
        "int printf(i8* message, ...);"
        ""
        "void print(f64 num) { printf(\"%lf\n\", num); }"
        ""
        "int main() {"
        "     print(3.43F);"
        "     print(3.43);"
        "     double d = 43.3434;"
        "     print(d);"
        "     float f = 42.544;"
        "     print(f);"
        "}"
    );
}