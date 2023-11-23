#pragma once

#include <types/TypeBase.h>

namespace dua
{

inline bool equal_types(TypeBase* t1, TypeBase* t2) {
    return (bool)t1 == (bool)t2 && (!t1 || *t1 == *t2);
}

struct Param {
    std::string name;
    TypeBase* type;
};

struct FunctionSignature
{
    TypeBase* return_type = nullptr;
    std::vector<Param> params;
    bool is_var_arg = false;

    bool operator==(const FunctionSignature& other)
    {
        if (!equal_types(return_type, other.return_type))
            return false;
        if (params.size() != other.params.size())
            return false;
        // The names can be different.
        for (size_t i = 0; i < params.size(); i++)
            if (!equal_types(params[i].type, other.params[i].type))
                return false;
        return is_var_arg == other.is_var_arg;
    }

    bool operator!=(const FunctionSignature& other) { return !(*this == other); }
};

}
