#include <iostream>
#include "EvaLLVM.h"

int main() {
    EvaLLVM module("MainModule");
    module.eval("int main() { return 5; }");
}
