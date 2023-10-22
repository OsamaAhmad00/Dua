#include <iostream>
#include "Compiler.h"

int main() {
    Compiler module("MainModule");
    module.compile("(scope"
                   "    (var x 15)"
                   "    (var y 16)"
                   "    (if (> x y)"
                   "        (if (== x y)"
                   "            (printf \"Then Then branch\n\")"
                   "            (printf \"Else Else branch\n\")"
                   "        )"
                   "        (printf \"Else branch\n\")"
                   "    )"
                   "    (if (< x y)"
                   "        (if (== x y)"
                   "            (printf \"Then Then branch\n\")"
                   "            (printf \"Then Else branch\n\")"
                   "        )"
                   "        (printf \"Else branch\n\")"
                   "    )"
                   ")"
    );
}