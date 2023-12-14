#include <AST/AssignmentExpressionNode.hpp>
#include <utils/ErrorReporting.hpp>

namespace dua
{

Value AssignmentExpressionNode::eval()
{
    auto lhs_res = lhs->eval();
    auto rhs_res = rhs->eval();
    auto llvm_type = lhs_res.type->llvm_type();

    if (auto infix = name_resolver().call_infix_operator(lhs_res, rhs_res, "Assignment"); !infix.is_null())
        return infix;

    if (rhs_res.type->llvm_type() != llvm_type) {
        Value alternative = typing_system().cast_value(rhs_res, lhs_res.type);
        if (alternative.is_null())
            report_error("Invalid assignment operation");
        rhs_res = alternative;
    }

    if (!llvm_type->isAggregateType())
        return compiler->create_value(builder().CreateStore(rhs_res.get(), lhs_res.memory_location), get_type());

    size_t n = -1;
    if (llvm_type->isArrayTy())
        n = llvm_type->getArrayNumElements();
    else
        report_internal_error("Not supported aggregate type");


    llvm::Type* element_type = llvm_type->getArrayElementType();
    std::vector<llvm::Value*> indices { builder().getInt32(0), builder().getInt32(0) };
    for (int i = 0; i < n; i++) {
        indices[1] = builder().getInt32(i);
        builder().CreateGEP(llvm_type, lhs_res.memory_location, indices);
        // Here, the target is the source, since we're loading from the target and storing into the source.
        llvm::Value* source_ptr = builder().CreateStructGEP(llvm_type, rhs_res.get(), 0);
        llvm::Value* target_ptr = builder().CreateStructGEP(llvm_type, lhs_res.memory_location, 0);
        llvm::Value* source = builder().CreateLoad(element_type, source_ptr);
        builder().CreateStore(source, target_ptr);
    }

    return rhs_res;
}

const Type* AssignmentExpressionNode::get_type() {
    return type = lhs->get_type();
}

}
