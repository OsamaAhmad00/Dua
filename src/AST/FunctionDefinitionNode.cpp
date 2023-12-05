#include <AST/function/FunctionDefinitionNode.hpp>
#include <types/VoidType.hpp>
#include <types/PointerType.hpp>
#include <utils/TextManipulation.hpp>

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
    auto& info = name_resolver().get_function(name);
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
        name_resolver().push_scope();
        auto& fields = name_resolver().get_class(current_class()->getName().str())->fields();
        auto self = function->args().begin();
        for (size_t i = 0; i < fields.size(); i++) {
            auto ptr = builder().CreateStructGEP(current_class(), self, i, fields[i].name);
            name_resolver().symbol_table.insert(fields[i].name, compiler->create_value(ptr, fields[i].type));
        }
    }

    // local variables
    name_resolver().push_scope();

    size_t i = 0;
    if (!info.param_names.empty() && info.param_names[0] == "self") {
        // FIXME don't allow parameters to be named "self"
        // The self variable doesn't need to be manipulated, thus,
        //  it doesn't need to be pushed on the stack. Moreover, if
        //  the self pointer is pushed to the stack, the variable
        //  on the stack would have a type of class** instead of class*.
        i++;
        auto type = dynamic_cast<const PointerType*>(info.type->param_types[0])->get_element_type();
        name_resolver().symbol_table.insert("self", compiler->create_value(function->args().begin(), type));
    }

    for (; i < info.param_names.size(); i++) {
        const auto& arg = function->args().begin() + i;
        arg->setName(info.param_names[i]);
        auto value = compiler->create_value(arg, info.type->param_types[i]);
        create_local_variable(info.param_names[i], info.type->param_types[i], &value);
    }

    std::string constructor_suffix = ".constructor";
    if (ends_with(name, constructor_suffix)) {
        // This is a constructor call.
        auto class_name = name.substr(0, name.size() - constructor_suffix.size());
        auto class_type = name_resolver().get_class(class_name);
        initialize_constructor(class_type);
    }

    body->eval();

    auto scope = name_resolver().pop_scope();
    scope.map.erase("self");
    name_resolver().destruct_all_variables(scope);

    if (current_class() != nullptr)
        name_resolver().pop_scope();

    if (info.type->return_type->llvm_type() == builder().getVoidTy())
        builder().CreateRetVoid();
    else {
        // TODO perform a more sophisticated analysis
        auto terminator = builder().GetInsertBlock()->getTerminator();
        if (!terminator || llvm::dyn_cast<llvm::ReturnInst>(terminator) == nullptr) {
            builder().CreateRet(info.type->return_type->default_value());
        }
    }

    builder().SetInsertPoint(old_block);
    current_function() = old_function;

    return function;
}

void FunctionDefinitionNode::initialize_constructor(const ClassType *class_type)
{
    // Call the constructors of the fields first, then the constructor of this class.
    // Constructors are called in the order of definition of fields in the class.
    auto& class_fields_args = name_resolver().get_fields_args(class_type->name);

    for (auto& field : class_type->fields())
    {
        bool found = false;
        auto type = class_type->get_field(field.name).type;
        auto ptr = class_type->get_field(name_resolver().symbol_table.get("self").ptr, field.name);
        std::vector<Value> args;

        // Check if there exists arguments passed to the constructor first
        for (auto& field_arg : class_fields_args) {
            if (field.name == field_arg.name) {
                found = true;
                args.resize(field_arg.args.size());
                for (size_t j = 0; j < args.size(); j++)
                    args[j] = compiler->create_value(field_arg.args[j]->eval(), field_arg.args[j]->get_type());
                break;
            }
        }

        // If not, check if there is a default initializer list
        if (!found) {
            args.insert(args.end(), field.default_args.begin(), field.default_args.end());
        }

        name_resolver().call_constructor(compiler->create_value(ptr, type), std::move(args));
    }
}

}
