#include <iostream>
#include "Compiler.h"

int main() {
    Compiler module("MainModule");
    module.compile("(scope"
                   "    (var hello 15)"
                   "    (printf \"Hello, world! %d\n\" hello)"
                   "    (printf \"Hello, again! %d\n\" (scope 1 2 3 4))"
                   ")"
    );
}