#include <iostream>
#include "Compiler.h"

int main() {
    Compiler module("MainModule");
    module.compile("(scope"
                   "    (var hello 15)"
                   "    (global world 16)"
                   "    (printf \"Local: %d\n\" hello)"
                   "    (printf \"Global: %d\n\" world)"
                   ")"
    );
}