#include "AST/variable/LocalVariableDefinitionNode.hpp"
#include "types/PointerType.hpp"

namespace dua
{

llvm::AllocaInst* LocalVariableDefinitionNode::eval()
{
    auto full_name = (current_class() ? current_class()->getName().str() + "::" : "") + name;

    if (initializer != nullptr && !args.empty())
        report_error("Can't have both an initializer and an initializer list (in " + full_name + ")");

    Value init_value;
    if (initializer) init_value = initializer->get_eval_value();

    if (current_function() != nullptr)
    {
        std::vector<Value> evaluated(args.size());
        for (int i = 0; i < args.size(); i++)
            evaluated[i] = compiler->create_value(args[i]->eval(), args[i]->get_type());
        auto ptr = initializer ? &init_value : nullptr;
        return create_local_variable(name, type, ptr, std::move(evaluated));
    }
    else if (current_class() == nullptr)
    {
        report_internal_error("Local variable definition in a non-local scope (the variable " + full_name + ")");
        return nullptr;
    }
    else
    {
        report_internal_error("Local variable definition in place of a field definition");
        return nullptr;
    }
}

}
