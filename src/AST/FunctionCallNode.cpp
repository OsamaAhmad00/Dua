#include <AST/function/FunctionCallNode.hpp>
#include <AST/lvalue/ClassFieldNode.hpp>
#include <AST/lvalue/LoadedLValueNode.hpp>
#include <types/PointerType.hpp>

namespace dua
{

FunctionType* get_function_type(ASTNode* func)
{
    auto type = func->get_cached_type();

    auto function_type = dynamic_cast<FunctionType*>(type);;

    if (function_type == nullptr) {
        auto function_pointer = dynamic_cast<PointerType *>(type);
        if (function_pointer != nullptr)
            function_type = dynamic_cast<FunctionType *>(function_pointer->get_element_type());
    }

    if (function_type == nullptr)
        report_error("Instantiating a call on a " + type->to_string() + ", which is not callable");

    return function_type;
}

llvm::CallInst* FunctionCallNode::eval()
{
    size_t n = args.size();

    auto loaded = dynamic_cast<LoadedLValueNode*>(func);
    auto field = dynamic_cast<ClassFieldNode*>(loaded->lvalue);
    bool is_method = field != nullptr && field->is_function();

    n += is_method;

    std::vector<llvm::Value*> llvm_args(n);

    for (size_t i = n - 1; i != ((size_t)-1 + is_method); i--)
        llvm_args[i] = args[i - is_method]->eval();

    if (is_method)
        llvm_args[0] = field->get_instance();

    return name_resolver().call_function(func->eval(), *get_function_type(func), llvm_args);
}

Type *FunctionCallNode::compute_type() {
    delete type;
    return type = get_function_type(func)->return_type->clone();
}

FunctionCallNode::~FunctionCallNode() {
    for (ASTNode* arg : args)
        delete arg;
    delete func;
}

}