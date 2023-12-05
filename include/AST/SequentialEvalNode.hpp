#pragma once

#include <AST/ASTNode.hpp>

namespace dua
{

class SequentialEvalNode : public ASTNode
{
    size_t return_node = 0;
    std::vector<ASTNode*> nodes;

public:

    SequentialEvalNode(ModuleCompiler* compiler, std::vector<ASTNode*> nodes, size_t return_node)
            : nodes(std::move(nodes)), return_node(return_node)
    {
        this->compiler = compiler;
        if (this->nodes.empty())
            report_internal_error("Sequential node constructed with empty list");
    }

    llvm::Value* eval() override {
        llvm::Value* result = nullptr;
        for (int i = 0; i < nodes.size(); i++) {
            if (i == return_node) {
                result = nodes[i]->eval();
            } else {
                nodes[i]->eval();
            }
        }
        return result;
    }

    const Type* get_type() override {
        return type = nodes[return_node]->get_type();
    }

};

}
