#pragma once

#include "LValueNode.hpp"
#include "types/PointerType.hpp"

namespace dua
{

class MallocNode : public LValueNode
{

    std::vector<ASTNode*> args;

public:

    MallocNode(ModuleCompiler* compiler, Type* type, std::vector<ASTNode*> args);

    llvm::Value* eval() override;

    Type* compute_type() override;

    Type* get_element_type() override;

    ~MallocNode() override;
};

}
