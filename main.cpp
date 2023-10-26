#include "Compiler.h"

int main() {
    Compiler module("MainModule");
    module.compile(
        ""
        "(fun main() -> int"
        "    (scope"
        "        (printf \"Hello, world!\n\")"
        "        (return 0)"
        "    )"
        ")"
    );
}