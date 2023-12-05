#pragma once

#include "AST/variable/VariableDefinitionNode.hpp"

namespace dua
{

class LocalVariableDefinitionNode : public VariableDefinitionNode
{
    llvm::Constant* get_constant(Value value, const Type* target_type, const std::string& field_name);

public:

    LocalVariableDefinitionNode(ModuleCompiler* compiler, std::string name,
                                const Type* type, ASTNode* initializer, std::vector<ASTNode*> args = {})
            : VariableDefinitionNode(compiler, std::move(name), type, initializer, std::move(args)) {}
    llvm::Value* eval() override;
};

}
