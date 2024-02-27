#include <AST/FreeNode.hpp>
#include "types/IntegerTypes.hpp"
#include "types/ArrayType.hpp"

namespace dua
{

FreeNode::FreeNode(ModuleCompiler* compiler, ASTNode* expr, bool is_array, bool call_destructors)
    : expr(expr), is_array(is_array), call_destructors(call_destructors)
{
    this->compiler = compiler;
    this->type = compiler->create_type<VoidType>();
}

Value FreeNode::eval()
{
    auto ptr = expr->eval();
    Value free_ptr = ptr;

    auto ptr_type = ptr.type->as<PointerType>();
    if (ptr_type == nullptr)
        compiler->report_error("The delete operator only accepts a pointer type, not a "
                               + expr->get_type()->to_string());

    auto null_condition_bb = compiler->create_basic_block("delete_is_null_condition");
    auto null_body_bb = compiler->create_basic_block("delete_not_null_body");
    auto end_bb = compiler->create_basic_block("delete_end");

    builder().CreateBr(null_condition_bb);
    builder().SetInsertPoint(null_condition_bb);

    auto is_null = builder().CreateIsNull(ptr.get());
    builder().CreateCondBr(is_null, end_bb, null_body_bb);

    builder().SetInsertPoint(null_body_bb);

    llvm::Value* count = builder().getInt64(1);
    if (is_array)
    {
        auto ptr_num = builder().CreatePtrToInt(ptr.get(), builder().getInt64Ty());
        ptr_num = builder().CreateSub(ptr_num, builder().getInt64(8));
        free_ptr.set(builder().CreateIntToPtr(ptr_num, ptr.type->llvm_type()));
        count = builder().CreateLoad(builder().getInt64Ty(), free_ptr.get());
    }

    if (call_destructors) {
        name_resolver().destruct_array(ptr, compiler->create_value(count, compiler->create_type<I64Type>()));
    }

    // Here, we pass the param type of free, regardless of the actual type of the expression, to
    // avoid the typing system complaining about the free function having the wrong pointer type
    auto param_type = name_resolver().get_function_no_overloading("free").type->param_types.front();
    free_ptr.type = param_type;
    name_resolver().call_function("free", { free_ptr });

    builder().SetInsertPoint(end_bb);

    return none_value();
}

}