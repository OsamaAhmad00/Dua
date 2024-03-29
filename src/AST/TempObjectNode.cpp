#include <AST/TempObjectNode.hpp>


namespace dua
{

Value TempObjectNode::eval()
{
    if (current_function() == nullptr)
        report_internal_error("Creation of a temp object in a non-local scope");

    auto evaluated = eval_args(args);

    auto ptr = compiler->create_local_variable("", type, nullptr, std::move(evaluated));
    auto result = compiler->create_value(type, ptr);

    // Inserting into the temp expressions map before setting memory_location to nullptr
    result.id = compiler->get_temp_expr_map_unused_id();
    compiler->insert_temp_expr(result);

    result.is_teleporting = true;

    return result;
}

const Type* TempObjectNode::get_type()
{
    return type;
}

}
