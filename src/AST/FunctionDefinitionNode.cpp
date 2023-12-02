#include <AST/function/FunctionDefinitionNode.h>
#include <types/VoidType.h>
#include <types/PointerType.h>
#include <utils/TextManipulation.h>

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
    auto& info = compiler->get_function(name);
    llvm::Function* function = module().getFunction(name);

    if (!function)
        report_internal_error("definition of an undeclared function");

    llvm::Function* old_function = current_function();
    llvm::BasicBlock* old_block = builder().GetInsertBlock();
    llvm::BasicBlock* current_block = create_basic_block("entry", function);
    builder().SetInsertPoint(current_block);
    current_function() = function;

    if (current_class() != nullptr) {
        // class fields
        compiler->push_scope();
        auto& fields = compiler->get_class(current_class()->getName().str())->fields();
        auto self = function->args().begin();
        for (size_t i = 0; i < fields.size(); i++) {
            auto ptr = builder().CreateStructGEP(current_class(), self, i, fields[i].name);
            symbol_table().insert(fields[i].name, { ptr, fields[i].type });
        }
    }

    // local variables
    compiler->push_scope();

    size_t i = 0;
    if (!info.param_names.empty() && info.param_names[0] == "self") {
        // The self variable doesn't need to be manipulated, thus,
        //  it doesn't need to be pushed on the stack. Moreover, if
        //  the self pointer is pushed to the stack, the variable
        //  on the stack would have a type of class** instead of class*.
        i++;
        auto type = dynamic_cast<PointerType*>(info.type.param_types[0])->get_element_type();
        symbol_table().insert("self", { function->args().begin(), type });
    }

    for (; i < info.param_names.size(); i++) {
        const auto& arg = function->args().begin() + i;
        arg->setName(info.param_names[i]);
        create_local_variable(info.param_names[i], info.type.param_types[i], arg);
    }

    std::string constructor_suffix = ".constructor";
    if (ends_with(name, constructor_suffix)) {
        // This is a constructor call
        auto class_name = name.substr(0, name.size() - constructor_suffix.size());
        auto class_type = compiler->get_class(class_name);
        // Call the constructors of the fields first, then the constructor of this class
        // Constructors are called in the order of definition.
        auto& class_fields_args = compiler->get_fields_args(class_type->name);
        for (const auto& field : class_fields_args) {
            // TODO don't search for the field twice, once for the type and once for the ptr
            auto type = class_type->get_field(field.name).type;
            auto ptr = class_type->get_field(symbol_table().get("self").ptr, field.name);
            std::vector<llvm::Value*> args(field.args.size());
            for (size_t j = 0; j < args.size(); j++)
                args[j] = field.args[j]->eval();
            compiler->call_constructor({ ptr, type }, std::move(args));
        }
    }

    body->eval();

    auto scope = compiler->pop_scope();
    scope.map.erase("self");
    compiler->destruct_all_variables(scope);

    if (current_class() != nullptr)
        compiler->pop_scope();

    if (info.type.return_type->llvm_type() == builder().getVoidTy())
        builder().CreateRetVoid();
    else {
        // TODO perform a more sophisticated analysis
        auto terminator = builder().GetInsertBlock()->getTerminator();
        if (!terminator || llvm::dyn_cast<llvm::ReturnInst>(terminator) == nullptr) {
            builder().CreateRet(info.type.return_type->default_value());
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
