#include "Compiler.h"

int main() {
    Compiler module("MainModule");
    module.compile("(scope"
                   "    (int result 1)"
                   "    (global int x 5)"
                   "    (while (> x 0)"
                   "        (scope "
                   "            (set result (* result x))"
                   "            (set x (- x 1))"
                   "        )"
                   "    )"
                   "    (printf \"%d\n\" result)"
                   ")"
    );
}