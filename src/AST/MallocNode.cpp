#include <AST/lvalue/MallocNode.hpp>
#include "types/IntegerTypes.hpp"
#include "types/ArrayType.hpp"

namespace dua
{

MallocNode::MallocNode(dua::ModuleCompiler *compiler, const Type *type, std::vector<ASTNode*> args, ASTNode* count, bool is_array, bool call_constructors)
    : args(std::move(args)), count(count), is_array(is_array), call_constructors(call_constructors)
{
    this->compiler = compiler;
    this->type = compiler->create_type<PointerType>(type);
}

Value MallocNode::eval()
{
    // is_array is needed. We can't just determine whether this is an array allocation or
    //  a single element allocation just by the count value. This is because new[1] should
    //  allocate space for size of the array. In other words, new X and new[1] X are different

    // This must be a pointer type
    auto ptr_type = get_type()->as<PointerType>();
    auto element_type = ptr_type->get_element_type();
    auto alloc_type = element_type->llvm_type();

    // TODO is setting the size to 1 is the best solution for non-sized struct types?
    size_t size = alloc_type->isSized() ? llvm::DataLayout(&module()).getTypeAllocSize(alloc_type) : 1;

    auto c = count->eval();
    auto as_i64 = c.cast_as(compiler->create_type<I64Type>(), false);
    if (as_i64.is_null())
        compiler->report_error("The type " + c.type->to_string()
            + " can't be used as the count for the new operator. (While allocating " + element_type->to_string() + ")");

    llvm::Value* bytes = builder().getInt64(size);
    bytes = builder().CreateMul(as_i64.get(), bytes);

    if (is_array) {
        // Reserve space for the number of elements as well
        bytes = builder().CreateAdd(bytes, builder().getInt64(8));
    }

    // Even though the returned type is I64*, we'll pretend it's a pointer type to the target type.
    //  LLVM typing system will allow substitution of any pointer in place of the other, but our
    //  typing system won't allow this, so this is a quick hack to avoid it complaining.
    auto pointer = name_resolver().call_function(
        "malloc",
         { compiler->create_value(bytes, compiler->create_type<I64Type>()) }
    );

    if (is_array)
    {
        // Embed the array size, and set the pointer to the start of the array
        builder().CreateStore(as_i64.get(), pointer.get());
        auto ptr_num = builder().CreatePtrToInt(pointer.get(), builder().getInt64Ty());
        ptr_num = builder().CreateAdd(ptr_num, builder().getInt64(8));
        pointer.set(builder().CreateIntToPtr(ptr_num, pointer.type->llvm_type()));
    }

    if (call_constructors)
    {
        std::vector<Value> evaluated(args.size());
        for (int i = 0; i < args.size(); i++)
            evaluated[i] = args[i]->eval();

        // Call the constructor for each element in a loop

        auto condition_bb = create_basic_block("malloc_init_condition", current_function());
        auto body_bb = create_basic_block("malloc_init_loop", current_function());
        auto end_bb = create_basic_block("malloc_init_end", current_function());

        auto counter = create_local_variable(".malloc_counter", compiler->create_type<I64Type>(), nullptr);
        builder().CreateBr(condition_bb);

        builder().SetInsertPoint(condition_bb);
        auto counter_val = builder().CreateLoad(builder().getInt64Ty(), counter);
        auto cmp = builder().CreateICmpEQ(counter_val, as_i64.get());
        builder().CreateCondBr(cmp, end_bb, body_bb);

        builder().SetInsertPoint(body_bb);
        auto array = compiler->create_type<ArrayType>(element_type, LONG_LONG_MAX);
        auto instance = builder().CreateGEP(
                array->llvm_type(),
                pointer.get(),
                { builder().getInt32(0), counter_val }
        );
        name_resolver().call_constructor(compiler->create_value(instance, element_type), evaluated);
        auto inc = builder().CreateAdd(counter_val, builder().getInt64(1));
        builder().CreateStore(inc, counter);
        builder().CreateBr(condition_bb);

        builder().SetInsertPoint(end_bb);
    }

    return compiler->create_value(pointer.get(), get_type());
}

const Type *MallocNode::get_type() {
    return type;
}

}