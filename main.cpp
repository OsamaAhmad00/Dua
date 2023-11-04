#include <Compiler.h>

int main() {
    Compiler module("MainModule");
    module.compile(
        ""
        "(varfun printf(str message) -> void)"
        ""
        "(fun main() -> int"
        "    (scope"
        "        (int _abc 5)"
        "        (str _message \"abc = %d\n\")"
        "        (global int abc _abc)"
        "        (global str message _message)"
        "        (printf message abc)"
        "        (set abc 3)"
        "        (printf _message abc)"
        "        (return 0)"
        "    )"
        ")"
    );
}