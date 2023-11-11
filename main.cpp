#include <ModuleCompiler.h>

int main()
{
    ModuleCompiler module(
        "MainModule",
        "i8 main(int x, i64 y, ...) { int a = 3; int b = 4; }"
    );
}