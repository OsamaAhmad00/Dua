#include "AST/function/FunctionRefNode.h"

namespace dua
{

llvm::Function* FunctionRefNode::eval()
{
    auto function = module().getFunction(name);
    if (function == nullptr) {
        report_error("Function " + name + " is not defined. Can't have a reference to an undefined function");
    }
    return function;
}

TypeBase* FunctionRefNode::compute_type()
{
    delete type;
    if (compiler->has_function(name))
        return type = compiler->get_function(name).type.clone();
    report_error("Function " + name + " is not defined");
    // Unreachable
    return nullptr;
}

}