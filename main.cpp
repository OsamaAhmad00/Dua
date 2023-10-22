#include <iostream>
#include "Compiler.h"

int main() {
    Compiler module("MainModule");
    module.compile("(scope"
                   "    (var hello 15)"
                   "    (global world 16)"
                   "    (printf \"%d\n%d\n\" hello world)"
                   "    (set hello 20)"
                   "    (set world 21)"
                   "    (printf \"%d\n%d\n\" hello world)"
                   ")"
    );
}