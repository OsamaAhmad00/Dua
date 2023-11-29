#include <AST/class/MethodCallNode.h>
#include "AST/lvalue/VariableNode.h"

namespace dua
{

MethodCallNode::MethodCallNode(dua::ModuleCompiler *compiler, LValueNode* instance,
        std::string name, std::vector<ASTNode *> args, bool panic_if_doesnt_exist, bool panic_if_not_class)
    : FunctionCallNode(compiler, std::move(name), std::move(args)), instance(instance),
      panic_if_doesnt_exist(panic_if_doesnt_exist), panic_if_not_class(panic_if_not_class)
{

}

llvm::CallInst* MethodCallNode::eval()
{
    if (!has_converted_to_method)
    {
        auto type = instance->get_element_type();
        auto casted = dynamic_cast<ClassType*>(type);
        if (casted != nullptr) {
            this->name = casted->name + '.' + this->name;
            this->args.insert(this->args.begin(), instance);
        } else if (panic_if_not_class) {
            report_internal_error("Calling methods on a non-class types");
        } else {
            return nullptr;
        }
        has_converted_to_method = true;
    }

    if (compiler->has_function(name)) {
        return FunctionCallNode::eval();
    } else {
        if (panic_if_doesnt_exist)
            report_error("The method " + name + " is not defined");
    }
    // Unreachable
    return nullptr;
}

}