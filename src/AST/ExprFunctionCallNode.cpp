#include <AST/function/ExprFunctionCallNode.hpp>
#include <AST/lvalue/ClassFieldNode.hpp>
#include <AST/lvalue/LoadedLValueNode.hpp>
#include <types/PointerType.hpp>

namespace dua
{

const FunctionType* get_function_type(ASTNode* func)
{
    auto type = func->get_type();

    auto function_type = dynamic_cast<const FunctionType*>(type);;

    if (function_type == nullptr) {
        auto function_pointer = dynamic_cast<const PointerType *>(type);
        if (function_pointer != nullptr)
            function_type = dynamic_cast<const FunctionType *>(function_pointer->get_element_type());
    }

    if (function_type == nullptr)
        report_error("Instantiating a call on a " + type->to_string() + ", which is not callable");

    return function_type;
}

Value ExprFunctionCallNode::eval()
{
    size_t n = args.size();

    auto loaded = dynamic_cast<LoadedLValueNode*>(func);
    auto field = dynamic_cast<ClassFieldNode*>(loaded->lvalue);
    bool is_method = field != nullptr && field->is_function();

    n += is_method;

    std::vector<Value> evaluated(n);

    for (size_t i = n - 1; i != ((size_t)-1 + is_method); i--) {
        auto arg = args[i - is_method];
        evaluated[i] = arg->eval();
    }

    if (is_method)
        evaluated[0] = field->eval_instance();

    return name_resolver().call_function(func->eval(), std::move(evaluated));
}

const Type *ExprFunctionCallNode::get_type() {
    if (type != nullptr) return type;
    return type = get_function_type(func)->return_type;
}

}