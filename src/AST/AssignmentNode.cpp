#include <AST/AssignmentExpressionNode.hpp>
#include <utils/ErrorReporting.hpp>

namespace dua
{

Value AssignmentExpressionNode::eval()
{
    auto ptr = lhs->eval();
    auto type = lhs->get_element_type();
    auto llvm_type = type->llvm_type();
    auto result = rhs->eval();

    if (result.type->llvm_type() != llvm_type) {
        Value alternative = typing_system().cast_value(result, type);
        if (alternative.is_null())
            report_error("Invalid assignment operation");
        result = alternative;
    }

    if (!llvm_type->isAggregateType())
        return compiler->create_value(builder().CreateStore(result.ptr, ptr.ptr), get_type());

    size_t n = -1;
    if (llvm_type->isArrayTy())
        n = llvm_type->getArrayNumElements();
    else
        report_internal_error("Not supported aggregate type");


    llvm::Type* element_type = llvm_type->getArrayElementType();
    std::vector<llvm::Value*> indices { builder().getInt32(0), builder().getInt32(0) };
    for (int i = 0; i < n; i++) {
        indices[1] = builder().getInt32(i);
        builder().CreateGEP(llvm_type, ptr.ptr, indices);
        // Here, the target is the source, since we're loading from the target and storing into the source.
        llvm::Value* source_ptr = builder().CreateStructGEP(llvm_type, result.ptr, 0);
        llvm::Value* target_ptr = builder().CreateStructGEP(llvm_type, ptr.ptr, 0);
        llvm::Value* source = builder().CreateLoad(element_type, source_ptr);
        builder().CreateStore(source, target_ptr);
    }

    return ptr;
}

const Type* AssignmentExpressionNode::get_type() {
    return type = lhs->get_type();
}

}
