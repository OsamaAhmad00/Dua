#include "AST/variable/LocalVariableDefinitionNode.hpp"
#include "types/PointerType.hpp"
#include "AST/lvalue/LoadedLValueNode.hpp"
#include "types/ReferenceType.hpp"

namespace dua
{

Value LocalVariableDefinitionNode::eval()
{
    auto full_name = (current_class() ? current_class()->getName().str() + "::" : "") + name;

    if (initializer != nullptr && !args.empty())
        report_error("Can't have both an initializer and an initializer list (in " + full_name + ")");

    // If it's a reference, just add a record to the symbol table and return
    if (auto ref = dynamic_cast<const ReferenceType*>(type); ref != nullptr) {
        if (auto loaded = dynamic_cast<LoadedLValueNode*>(initializer); loaded != nullptr) {
            auto result = loaded->lvalue->eval();
            name_resolver().symbol_table.insert(name, result);
            return result;
        } else {
            report_error("Can't have a reference type to a non-lvalue expression");
        }
    }

    // TODO don't evaluate if not going to be used
    Value init_value;
    if (initializer) init_value = initializer->eval();

    if (current_function() != nullptr)
    {
        std::vector<Value> evaluated(args.size());
        for (int i = 0; i < args.size(); i++)
            evaluated[i] = args[i]->eval();
        auto ptr = initializer ? &init_value : nullptr;
        auto alloc = create_local_variable(name, type, ptr, std::move(evaluated));
        return compiler->create_value(alloc, get_type());
    }
    else if (current_class() == nullptr)
    {
        report_internal_error("Local variable definition in a non-local scope (the variable " + full_name + ")");
        return none_value();
    }
    else
    {
        report_internal_error("Local variable definition in place of a field definition");
        return none_value();
    }
}

}
