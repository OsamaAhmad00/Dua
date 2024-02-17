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

    if (call_destructors)
    {
        auto condition_bb = compiler->create_basic_block("delete_destruct_condition");
        auto body_bb = compiler->create_basic_block("delete_destruct_loop");
        auto destruct_end_bb = compiler->create_basic_block("delete_destruct_end");

        auto counter = compiler->create_local_variable(".delete_counter", compiler->create_type<I64Type>(), nullptr);
        builder().CreateBr(condition_bb);

        builder().SetInsertPoint(condition_bb);
        auto counter_val = builder().CreateLoad(builder().getInt64Ty(), counter);
        auto cmp = builder().CreateICmpEQ(counter_val, count);
        builder().CreateCondBr(cmp, destruct_end_bb, body_bb);

        builder().SetInsertPoint(body_bb);
        auto element_type = ptr_type->get_element_type();
        auto array = compiler->create_type<ArrayType>(element_type, LONG_LONG_MAX);
        auto instance = builder().CreateGEP(
            array->llvm_type(),
            ptr.get(),
            { builder().getInt32(0), counter_val }
        );
        name_resolver().call_destructor(compiler->create_value(instance, element_type));
        auto inc = builder().CreateAdd(counter_val, builder().getInt64(1));
        builder().CreateStore(inc, counter);
        builder().CreateBr(condition_bb);

        builder().SetInsertPoint(destruct_end_bb);
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