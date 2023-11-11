#include <ModuleCompiler.h>

int main()
{
    ModuleCompiler module(
        "MainModule",
        "long x = 3; i32 y = 55S; i8 z = 1L;"
    );
}