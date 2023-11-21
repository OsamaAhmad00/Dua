#include <AST/AssignmentExpressionNode.h>
#include <utils/ErrorReporting.h>

namespace dua
{

llvm::Value* AssignmentExpressionNode::eval()
{
    llvm::Value* ptr = lhs->eval();
    llvm::Type* type = lhs->get_cached_type()->llvm_type();
    llvm::Value* result = rhs->eval();

    if (result->getType() != type) {
        llvm::Value* alternative = compiler->cast_value(result, type);
        if (alternative == nullptr)
            report_error("Invalid assignment operation");
        result = alternative;
    }

    if (!type->isAggregateType())
        return builder().CreateStore(result, ptr);

    size_t n = -1;
    if (type->isArrayTy())
        n = type->getArrayNumElements();
    else
        report_internal_error("Not supported aggregate type");


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

TypeBase *AssignmentExpressionNode::compute_type() {
    delete type;
    return type = lhs->get_cached_type()->clone();
}

AssignmentExpressionNode::~AssignmentExpressionNode()
{
    delete lhs;
    delete rhs;
}

}
