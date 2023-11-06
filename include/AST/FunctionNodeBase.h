#pragma once

#include <AST/ASTNode.h>
#include <AST/terminals/ValueNode.h>


struct FunctionNodeBase : public ASTNode
{
    struct Param {
        std::string name;
        std::string type;
    };
    struct FunctionSignature {
        std::string name;
        std::string return_type;
        std::vector<Param> params;
        bool is_var_arg = false;
    };
};