#include <iostream>
#include "Compiler.h"

int main() {
    Compiler module("MainModule");
    module.compile("(scope"
                   "    (var hello 15)"
                   "    (global world 16)"
                   "    (printf \"Local: %d\n\" hello)"
                   "    (scope "
                   "         (var inner_local 22)"
                   "         (global inner_global 33)"
                   "         (printf \"inner_local: %d\n\" inner_local)"
                   "         (printf \"inner_global: %d\n\" inner_global)"
                   "    )"
//                   "    (printf \"inner_local: %d\n\" inner_local)"
                   "    (printf \"inner_global: %d\n\" inner_global)"
                   "    (printf \"Global: %d\n\" world)"
                   ")"
    );
}