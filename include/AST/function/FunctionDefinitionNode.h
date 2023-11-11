#pragma once

#include <AST/function/FunctionNodeBase.h>

class FunctionDefinitionNode : public FunctionNodeBase
{
    FunctionSignature signature;
    ASTNode* body;  // Can be nullptr in case of declaration

    llvm::Function* define_function();
    llvm::Function* declare_function();

public:

    FunctionDefinitionNode(ModuleCompiler* compiler, FunctionSignature signature, ASTNode* body)
        : signature(std::move(signature)), body(body) { this->compiler = compiler; }
    llvm::Function* eval() override;
    ~FunctionDefinitionNode() override;
};