#pragma once

#include <AST/FunctionNodeBase.h>

class FunctionDefinitionNode : public FunctionNodeBase
{
    FunctionSignature signature;
    ASTNode* body;  // Can be nullptr in case of declaration

    llvm::Function* define_function(const FunctionSignature& signature, ASTNode* body);
    llvm::Function* declare_function(const FunctionSignature& signature);

public:

    FunctionDefinitionNode(FunctionSignature signature, ASTNode* body)
        : signature(std::move(signature)), body(body) {}
    llvm::Function* eval() override;
    ~FunctionDefinitionNode() override;
};