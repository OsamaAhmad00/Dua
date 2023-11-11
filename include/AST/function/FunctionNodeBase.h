#pragma once

#include <AST/ASTNode.h>
#include <AST/terminals/ValueNode.h>


struct FunctionNodeBase : public ASTNode
{
    struct Param {
        std::string name;
        TypeBase* type;

        ~Param() { delete type; }
    };
    struct FunctionSignature {
        std::string name;
        TypeBase* return_type = nullptr;
        std::vector<Param> params;
        bool is_var_arg = false;

        ~FunctionSignature() { delete return_type; }
    };
};