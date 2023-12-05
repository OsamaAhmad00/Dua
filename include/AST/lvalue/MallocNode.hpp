#pragma once

#include "LValueNode.hpp"
#include "types/PointerType.hpp"

namespace dua
{

class MallocNode : public LValueNode
{

    std::vector<ASTNode*> args;

public:

    MallocNode(ModuleCompiler* compiler, const Type* type, std::vector<ASTNode*> args);

    llvm::Value* eval() override;

    const Type* get_type() override;

    const Type* get_element_type() override;
};

}
