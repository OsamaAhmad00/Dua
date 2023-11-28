#pragma once

#include <AST/function/FunctionCallNode.h>
#include <FunctionInfo.h>
#include "AST/lvalue/LValueNode.h"

namespace dua
{

class MethodCallNode : public FunctionCallNode
{
    bool has_converted_to_method = false;
    bool panic_if_doesnt_exist;
    bool panic_if_not_class;
    LValueNode* instance;

public:

    MethodCallNode(ModuleCompiler* compiler, LValueNode* instance, std::string name,
                   std::vector<ASTNode*> args = {}, bool panic_if_doesnt_exist = true, bool panic_if_not_class = true);

    llvm::CallInst* eval() override;
};

}
