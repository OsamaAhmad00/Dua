#include "Compiler.h"

int main() {
    Compiler module("MainModule");
    module.compile(
        ""
        "(varfun printf(str message) -> void)"
        ""
        "(fun multiply(int a, int b) -> int"
        "    (return (* a b))"
        ")"
        ""
        "(fun main() -> int"
        "    (scope"
        "        (printf \"5 * 12 = %d\n\" (multiply 5 12))"
        "        (return 0)"
        "    )"
        ")"
    );
}