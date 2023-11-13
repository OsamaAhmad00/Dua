#include <ModuleCompiler.h>

int main()
{
    ModuleCompiler module(
        "MainModule", ""
        "int main() {"
        "   return when {"
        "       1 -> 2,"
        "       2 -> 4,"
        "       5 -> when { 6 -> 7, else -> 8 },"
        "       9 -> 10,"
        "       else -> 11"
        "   };"
        "}"
    );
}