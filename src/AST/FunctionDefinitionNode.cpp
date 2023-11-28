#include <AST/function/FunctionDefinitionNode.h>
#include <types/VoidType.h>
#include "types/PointerType.h"

namespace dua
{

FunctionDefinitionNode::FunctionDefinitionNode(dua::ModuleCompiler *compiler, std::string name, dua::ASTNode *body)
        : name(std::move(name)), body(body)
{
    this->compiler = compiler;

    if (current_function() != nullptr)
        report_internal_error("Nested functions are not allowed");
}

llvm::Function *FunctionDefinitionNode::eval()
{
    // The declaration logic is moved to the parser,
    //  so that all functions are visible everywhere
    //  across the module, regardless of the order
    //  of declaration/definition.
    if (body == nullptr)
        return module().getFunction(name);
    return define_function();
}

llvm::Function* FunctionDefinitionNode::define_function()
{
    auto& signature = compiler->get_function(name);
    llvm::Function* function = module().getFunction(name);

    if (!function)
        report_internal_error("Use of an undeclared function");

    llvm::Function* old_function = current_function();
    llvm::BasicBlock* old_block = builder().GetInsertBlock();
    llvm::BasicBlock* current_block = create_basic_block("entry", function);
    builder().SetInsertPoint(current_block);
    current_function() = function;

    if (current_class() != nullptr) {
        // class fields
        symbol_table().push_scope();
        auto& fields = compiler->get_class(current_class()->getName().str())->fields();
        auto self = function->args().begin();
        for (size_t i = 0; i < fields.size(); i++) {
            auto ptr = builder().CreateStructGEP(current_class(), self, i, fields[i].name);
            symbol_table().insert(fields[i].name, { ptr, fields[i].type });
        }
    }

    // local variables
    symbol_table().push_scope();

    size_t i = 0;
    if (!signature.params.empty() && signature.params[0].name == "self") {
        // The self variable doesn't need to be manipulated, thus,
        //  it doesn't need to be pushed on the stack. Moreover, if
        //  the self pointer is pushed to the stack, the variable
        //  on the stack would have a type of class** instead of class*.
        i++;
        auto type = dynamic_cast<PointerType*>(signature.params[0].type)->get_element_type();
        symbol_table().insert("self", { function->args().begin(), type });
    }

    for (; i < signature.params.size(); i++) {
        const auto& arg = function->args().begin() + i;
        auto& param = signature.params[i];
        arg->setName(param.name);
        create_local_variable(param.name, param.type, arg);
    }

    body->eval();

    auto scope = symbol_table().pop_scope();
    scope.map.erase("self");
    compiler->destruct_all_variables(scope);

    if (current_class() != nullptr)
        symbol_table().pop_scope();

    if (signature.return_type->llvm_type() == builder().getVoidTy())
        builder().CreateRetVoid();
    else {
        // TODO perform a more sophisticated analysis
        auto terminator = builder().GetInsertBlock()->getTerminator();
        if (!terminator || llvm::dyn_cast<llvm::ReturnInst>(terminator) == nullptr) {
            builder().CreateRet(signature.return_type->default_value());
        }
    }

    builder().SetInsertPoint(old_block);
    current_function() = old_function;

    return function;
}

FunctionDefinitionNode::~FunctionDefinitionNode()
{
    delete body;
}

}
