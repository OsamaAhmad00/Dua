#include <AST/function/FunctionDefinitionNode.hpp>
#include <types/VoidType.hpp>
#include <types/PointerType.hpp>
#include <utils/TextManipulation.hpp>
#include "types/ReferenceType.hpp"

namespace dua
{

FunctionDefinitionNode::FunctionDefinitionNode(dua::ModuleCompiler *compiler,
               std::string name, dua::ASTNode *body, const FunctionType* function_type)
        : name(std::move(name)), body(body), function_type(function_type)
{
    this->compiler = compiler;

    if (current_function() != nullptr)
        report_internal_error("Nested functions are not allowed");
}

Value FunctionDefinitionNode::eval()
{
    // The declaration logic is moved to the parser,
    //  so that all functions are visible everywhere
    //  across the module, regardless of the order
    //  of declaration/definition.
    if (body == nullptr)
        return compiler->create_value(module().getFunction(name), get_type());
    return define_function();
}

Value FunctionDefinitionNode::define_function()
{
    auto& info = name_resolver().get_function_no_overloading(name);
    llvm::Function* function = module().getFunction(name);

    if (!function)
        report_internal_error("definition of an undeclared function");

    if (!function->empty())
        report_error("Redefinition of the function " + name);

    llvm::Function* old_function = current_function();
    llvm::BasicBlock* old_block = builder().GetInsertBlock();
    llvm::BasicBlock* current_block = create_basic_block("entry", function);
    builder().SetInsertPoint(current_block);
    current_function() = function;

    bool is_method = false;

    if (current_class() != nullptr)
    {
        // Make class fields accessible from within the method
        name_resolver().push_scope();
        auto class_name = current_class()->getName().str();
        auto class_type = name_resolver().get_class(class_name);
        auto& fields = class_type->fields();
        auto first_arg = function->args().begin();
        auto class_value = compiler->create_value(first_arg, class_type);
        for (size_t i = 0; i < fields.size(); i++) {
            auto f = class_type->get_field(class_value, i);
            name_resolver().symbol_table.insert(fields[i].name, f);
        }
    }

    // For parameters and local variables. This will
    //  shadow the names of the fields in case of collisions.
    name_resolver().push_scope();

    if (current_class() != nullptr)
    {
        // The self variable doesn't need to be manipulated, thus,
        //  it doesn't need to be pushed on the stack. Moreover, if
        //  the self pointer is pushed to the stack, the variable
        //  on the stack would have a type of class** instead of class*.
        auto type = dynamic_cast<const ReferenceType*>(info.type->param_types[0])->get_element_type();
        name_resolver().symbol_table.insert("self", compiler->create_value(function->args().begin(), type));

        for (size_t j = 1; j < info.param_names.size(); j++)
            if (info.param_names[j] == "self")
                report_error("The parameter name 'self' is reserved in class methods (the parameter number "
                    + std::to_string(j + 1) + " of the method " + name);

        is_method = true;
    }

    for (size_t i = is_method; i < info.param_names.size(); i++) {
        const auto& arg = function->args().begin() + i;
        arg->setName(info.param_names[i]);
        auto type = info.type->param_types[i];
        if (auto ref = dynamic_cast<const ReferenceType*>(type); ref != nullptr) {
            // If it's a reference type, the llvm type of the parameter will be a pointer type,
            // which will hold the address of the variable, just like the result of an alloca.
            auto value = compiler->create_value(arg, ref);
            name_resolver().symbol_table.insert(info.param_names[i], value);
        } else {
            auto value = compiler->create_value(arg, type);
            create_local_variable(info.param_names[i], type, &value);
        }
    }

    size_t pos;
    for (auto& ctor : std::vector<std::string>{ ".constructor.", ".=constructor." }) {
        pos = name.find(ctor);
        if (pos != std::string::npos) break;
    }

    if (pos != std::string::npos) {
        // This is a constructor call.
        auto class_name = name.substr(0, pos);
        auto class_type = name_resolver().get_class(class_name);
        initialize_constructor(class_type);
    }

    body->eval();

    auto scope = name_resolver().pop_scope();
    // Don't want to destroy the self reference along with the parameters
    scope.map.erase("self");
    name_resolver().destruct_all_variables(scope);

    if (current_class() != nullptr)
        name_resolver().pop_scope();

    // Implicit return values
    if (info.type->return_type->llvm_type() == builder().getVoidTy())
        builder().CreateRetVoid();
    else {
        // TODO perform a more sophisticated analysis
        auto terminator = builder().GetInsertBlock()->getTerminator();
        if (!terminator || llvm::dyn_cast<llvm::ReturnInst>(terminator) == nullptr) {
            builder().CreateRet(info.type->return_type->default_value().get());
        }
    }

    builder().SetInsertPoint(old_block);
    current_function() = old_function;

    return compiler->create_value(function, get_type());
}

void FunctionDefinitionNode::initialize_constructor(const ClassType *class_type)
{
    // Call the constructors of the fields first, then the constructor of this class.
    // Constructors are called in the order of definition of fields in the class.
    auto& class_fields_args = name_resolver().get_fields_args(name);

    auto self = name_resolver().symbol_table.get("self");
    for (auto& field : class_type->fields())
    {
        bool found = false;
        auto type = class_type->get_field(field.name).type;
        auto ptr = class_type->get_field(self, field.name).get();
        std::vector<Value> args;

        // Check if there exists arguments passed to the constructor first
        for (auto& field_arg : class_fields_args) {
            if (field.name == field_arg.name) {
                found = true;
                args.resize(field_arg.args.size());
                for (size_t j = 0; j < args.size(); j++)
                    args[j] = field_arg.args[j]->eval();
                break;
            }
        }

        // If not, check if there is a default initializer list
        if (!found) {
            args.insert(args.end(), field.default_args.begin(), field.default_args.end());
        }

        // If no constructor initializer list, no default initializer list,
        //  initialize it with the default value if exists, or with the type
        //  default value.
        // If it's not assigned a value, then it has an empty arg list
        bool has_empty_args = field.default_value == nullptr;
        auto instance = compiler->create_value(ptr, type);
        if (!has_empty_args && args.empty()) {
            auto arg = compiler->create_value(field.default_value, field.type);
            name_resolver().call_copy_constructor(instance, arg);
        } else {
            name_resolver().call_constructor(instance, std::move(args));
        }
    }
}

}
