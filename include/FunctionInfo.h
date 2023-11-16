#pragma once

#include <types/TypeBase.h>

namespace dua
{

struct Param {
    std::string name;
    TypeBase* type;
};

struct FunctionSignature {
    TypeBase* return_type = nullptr;
    std::vector<Param> params;
    bool is_var_arg = false;
};

}
