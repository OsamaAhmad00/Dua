#include <AST/TempVariableNode.hpp>


namespace dua
{

size_t TempVariableNode::counter = 0;

Value TempVariableNode::eval()
{
    std::vector<Value> eval_args(args.size());
    for (size_t i = 0; i < args.size(); i++)
        eval_args[i] = args[i]->eval();
    name = ".temp_" + std::to_string(counter++);
    auto variable = create_local_variable(name, type, nullptr, std::move(eval_args));
    return compiler->create_value(type, variable);
}

const Type* TempVariableNode::get_type()
{
    return type;
}

void TempVariableNode::set_teleported()
{
    name_resolver().symbol_table.remove_last_occurrence_of(name);
}

}
