#include <iostream>
#include "Compiler.h"

int main() {
    Compiler module("MainModule");
    module.compile("(scope"
                   "    (printf \"Hello, world! %d\n\" 33)"
                   "    (printf \"Hello, again! %d\n\" 44)"
                   ")"
    );
}