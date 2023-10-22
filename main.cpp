#include <iostream>
#include "Compiler.h"

int main() {
    Compiler module("MainModule");
    module.compile("(scope"
                   "    (printf \"%d\n\" (+  1 2 3 4 5))"
                   "    (printf \"%d\n\" (-  1 2 3 4 5))"
                   "    (printf \"%d\n\" (*  1 2 3 4 5))"
                   "    (printf \"%d\n\" (/  5 2 1))"
                   "    (printf \"%d\n\" (<  1 2))"
                   "    (printf \"%d\n\" (>  1 2))"
                   "    (printf \"%d\n\" (<= 1 2))"
                   "    (printf \"%d\n\" (>= 1 2))"
                   "    (printf \"%d\n\" (== 1 2))"
                   "    (printf \"%d\n\" (!= 1 2))"
                   ")"
    );
}