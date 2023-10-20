#include <iostream>
#include "Compiler.h"

int main() {
    Compiler module("MainModule");
    module.compile("(printf \"Hello, world! %d\n\" 42)");
}