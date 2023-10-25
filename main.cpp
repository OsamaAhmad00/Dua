#include "Compiler.h"

int main() {
    Compiler module("MainModule");
    module.compile("(scope"
                   "    (int result 1)"
                   "    (global int x 5)"
                   "    (global str template \"%d\n\")"
                   "    (while (> x 0)"
                   "        (scope "
                   "            (int counter x)"
                   "            (printf template counter)"
                   "            (set result (* result x))"
                   "            (set x (- x 1))"
                   "        )"
                   "    )"
                   "    (printf template result)"
                   ")"
    );
}