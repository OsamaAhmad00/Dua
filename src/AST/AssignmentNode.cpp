#include <AST/AssignmentExpressionNode.h>
#include <Utils.h>

llvm::Value* AssignmentExpressionNode::eval()
{
    auto [ptr, type] = symbol_table().get(lhs);
    llvm::Value* result = rhs->eval();

    if (result->getType() != type) {
        llvm::Value* alternative = cast_value(result, type, builder());
        if (alternative == nullptr)
            throw std::runtime_error("Invalid assignment operation");
        result = alternative;
    }

    if (!type->isAggregateType())
        return builder().CreateStore(result, ptr);

    size_t n;
    if (type->isArrayTy())
        n = type->getArrayNumElements();
    else
        throw std::runtime_error("Not supported aggregate type");


    llvm::Type* element_type = type->getArrayElementType();
    std::vector<llvm::Value*> indices { builder().getInt32(0), builder().getInt32(0) };
    for (int i = 0; i < n; i++) {
        indices[1] = builder().getInt32(i);
        builder().CreateGEP(type, ptr, indices);
        // Here, the target is the source, since we're loading from the target and storing into the source.
        llvm::Value* source_ptr = builder().CreateStructGEP(type, result, 0);
        llvm::Value* target_ptr = builder().CreateStructGEP(type, ptr, 0);
        llvm::Value* source = builder().CreateLoad(element_type, source_ptr);
        builder().CreateStore(source, target_ptr);
    }

    return ptr;
}

AssignmentExpressionNode::~AssignmentExpressionNode()
{
    delete rhs;
}