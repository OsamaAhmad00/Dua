#include <AST/function/FunctionDefinitionNode.hpp>
#include <types/VoidType.hpp>
#include <utils/TextManipulation.hpp>
#include "types/ReferenceType.hpp"
#include "types/PointerType.hpp"
#include <AST/types/TypeAliasNode.hpp>

namespace dua
{

FunctionDefinitionNode::FunctionDefinitionNode(dua::ModuleCompiler *compiler,
               std::string name, dua::ASTNode *body, const FunctionType* function_type, bool nomangle, size_t template_param_count, bool is_operator, bool is_static)
        : name(std::move(name)), body(body), function_type(function_type), nomangle(nomangle), template_param_count(template_param_count), is_operator(is_operator), is_static(is_static)
{
    this->compiler = compiler;

    if (current_function() != nullptr)
        compiler->report_internal_error("Nested functions are not allowed");
}

Value FunctionDefinitionNode::eval()
{
    // The declaration logic is moved to the parser,
    //  so that all functions are visible everywhere
    //  across the module, regardless of the order
    //  of declaration/definition.

    if (template_param_count >= 0)
        return none_value();

    set_full_name();

    if (body == nullptr)
        return compiler->create_value(module().getFunction(name), get_type());

    return define_function();
}

Value FunctionDefinitionNode::define_function()
{
    auto& info = name_resolver().get_function_no_overloading(name);
    llvm::Function* function = module().getFunction(name);

    if (!function)
        compiler->report_internal_error("definition of an undeclared function");

    if (!function->empty())
        compiler->report_error("Redefinition of the function " + name);

    if (is_static)
        function->setLinkage(llvm::Function::InternalLinkage);

    llvm::Function* old_function = current_function();
    llvm::BasicBlock* old_block = builder().GetInsertBlock();
    llvm::BasicBlock* current_block = compiler->create_basic_block("entry", function);
    builder().SetInsertPoint(current_block);
    current_function() = function;

    bool is_method = false;

    if (current_class() != nullptr)
    {
        is_method = true;

        // Create an extra counter to avoid interfering
        // with the counter of the current method
        compiler->push_scope_counter();

        // Make class fields accessible from within the method
        compiler->push_scope();

        auto class_name = current_class()->getName().str();
        auto class_type = name_resolver().get_class(class_name);

        auto& fields = class_type->fields();

        auto self = function->args().begin();
        auto class_value = compiler->create_value(self, class_type);

        auto& aliases = name_resolver().get_class_aliases(class_name);
        for (auto node : aliases)
            node->eval();

        for (size_t i = 0; i < fields.size(); i++) {
            auto f = class_type->get_field(class_value, i);
            if (auto ref = f.type->as<ReferenceType>(); ref != nullptr) {
                // We turn the field into an unallocated reference here
                // Reference fields has two indirections instead of one.
                //  Normal references would just hold the address of the
                //  referenced variable, and getting the value would be
                //  a matter of dereferencing the address. For reference
                //  fields, first, dereference the offset in the class to
                //  get the pointer, then dereference the pointer to get
                //  the value. This is why we store into the memory_location,
                //  to get two dereferences instead of one
                f.memory_location = f.get();
                f.set(nullptr);
                f.get();
                f.type = ref->get_unallocated();
            }
            name_resolver().symbol_table.insert(fields[i].name, f);
        }
    }

    compiler->push_scope_counter();

    // For parameters and local variables. This will
    //  shadow the names of the fields in case of collisions.
    compiler->push_scope();

    if (is_method)
    {
        // The self variable doesn't need to be manipulated, thus,
        //  it doesn't need to be pushed on the stack.
        auto type = info.type->param_types[0];
        auto ref = type->as<ReferenceType>();
        assert(ref != nullptr);
        // Reference parameters are always passed as allocated
        type = ref->get_unallocated();
        auto self = function->args().begin();
        self->setName("self");
        name_resolver().symbol_table.insert("self", compiler->create_value(self, type));

        for (size_t j = 1; j < info.param_names.size(); j++)
            if (info.param_names[j] == "self")
                compiler->report_error("The parameter name 'self' is reserved in class methods (the parameter number "
                    + std::to_string(j + 1) + " of the method " + name);
    }

    bool is_main = !is_method && name == "main";

    if (!is_static && !is_main) {
        // Set the comdat selection kind to any, to avoid
        //  redefinition errors while linking.
        auto comdat = module().getOrInsertComdat(name);
        comdat->setSelectionKind(llvm::Comdat::SelectionKind::Any);
        function->setComdat(comdat);
    }

    for (size_t i = is_method; i < info.param_names.size(); i++) {
        const auto& arg = function->args().begin() + i;
        arg->setName(info.param_names[i]);
        auto type = info.type->param_types[i];
        if (auto ref = type->as<ReferenceType>(); ref != nullptr) {
            // If it's a reference type, the llvm type of the parameter will be a pointer type,
            //  which will hold the address of the variable, just like the result of an alloca.
            // We turn the reference into an unallocated reference here
            // Reference arguments are passed as allocated references
            auto value = compiler->create_value(arg, ref->get_unallocated());
            name_resolver().symbol_table.insert(info.param_names[i], value);
        } else {
            auto value = compiler->create_value(arg, type);
            // All arguments will teleport, because their copy constructor is already
            //  called (if needed).
            value.is_teleporting = true;
            compiler->create_local_variable(info.param_names[i], type, &value, {});
        }
    }

    size_t pos;
    for (auto& ctor : { ".constructor(", ".=constructor(" }) {
        pos = name.find(ctor);
        if (pos != std::string::npos) break;
    }

    if (pos != std::string::npos)
    {
        // This is a constructor call.
        auto class_name = name.substr(0, pos);
        assert(name_resolver().has_class(class_name));
        auto class_type = name_resolver().get_class(class_name);
        construct_fields(class_type);
    }

    body->eval();

    pos = name.find(".destructor(");
    if (pos != std::string::npos)
    {
        // This is a destructor call.
        auto class_name = name.substr(0, pos);
        assert(name_resolver().has_class(class_name));
        auto class_type = name_resolver().get_class(class_name);
        destruct_fields(class_type);
    }

    // Implicit return values
    size_t created_default_values = 0;
    bool is_void = info.type->return_type->as<VoidType>() != nullptr;
    for (auto& basic_block : *function)
    {
        auto terminator = basic_block.getTerminator();
        if (terminator == nullptr)
        {
            // There is no terminator for this basic block

            builder().SetInsertPoint(&basic_block);

            compiler->destruct_function_scope();

            if (is_void) {
                builder().CreateRetVoid();
            } else {
                created_default_values++;
                builder().CreateRet(info.type->return_type->default_value().get());
            }
        }
    }

    // It's ok for the main function to not return a value explicitly
    if (created_default_values && name != "main") {
        auto count = std::to_string(created_default_values);
        if (info.type->return_type->as<ClassType>() != nullptr) {
            compiler->report_error("The function " + name + " doesn't return a value at " + count +
                " terminal positions. Can't return a default value instead since the return type is of the class type "
                + info.type->return_type->to_string());
        } else {
            compiler->report_warning("The function " + name + " doesn't return a value at "
                                     + count + " terminal positions. Returning the default value instead");
        }
    }

    // Since each node takes care of the scopes it has created,
    //  at this point, there must be only once scope for the
    //  function (and one for the fields in case of a method)
    // Note that we pop the scope first then the counter.
    compiler->pop_scope();
    compiler->pop_scope_counter();

    if (current_class() != nullptr) {
        compiler->pop_scope();
        compiler->pop_scope_counter();
    }

    builder().SetInsertPoint(old_block);
    current_function() = old_function;

    return none_value();
}

void FunctionDefinitionNode::construct_fields(const ClassType *class_type)
{
    // Call the constructors of the fields first, then the constructor of this class.
    // Constructors are called in the order of definition of fields in the class.
    auto& class_fields_args = name_resolver().get_fields_args(name);

    for (size_t i = 0; i < class_fields_args.size(); i++)
    {
        if (class_fields_args[i].name != "Super")
        {
            bool found = false;
            for (auto &field: class_type->fields()) {
                if (field.name == class_fields_args[i].name) {
                    found = true;
                    break;
                }
            }

            if (!found) {
                compiler->report_error("Can't initialize the field " + class_fields_args[i].name + " in the constructor " + name +
                             ", which is not a field of the class " + class_type->name);
            }
        }

        for (size_t j = i + 1; j < class_fields_args.size(); j++) {
            if (class_fields_args[i].name == class_fields_args[j].name) {
                if (class_fields_args[i].name == "Super") {
                    compiler->report_error("Can't have more than one call to the constructor of the super-class (in the constructor " + name + ")");
                } else {
                    compiler->report_error("The field argument " + class_fields_args[i].name + " is present more than once in the constructor " + name);
                }
            }
        }
    }

    auto self = name_resolver().symbol_table.get("self");

    if (class_type->name != "Object")
    {
        // Before any initialization, call the parent constructor first
        std::vector<Value> parent_args;
        auto parent = compiler->get_name_resolver().parent_classes[class_type->name];
        auto parent_ptr = compiler->create_type<PointerType>(parent);
        // TODO Change this way of dealing with references and pointers
        // The self reference is of a reference type, and when converted to an LLVM type,
        //  it's represented just as the type it's pointing to. LLVM typing typing systems
        //  expect a pointer type, thus, we set the type as a pointer, cast it, the set it
        //  back again as a reference
        auto self_as_parent = self;
        self_as_parent.type = compiler->create_type<PointerType>(class_type);
        self_as_parent = self_as_parent.cast_as(parent_ptr);
        self_as_parent.type = parent;

        for (auto& field_args : class_fields_args)
        {
            if (field_args.name == "Super")
            {
                parent_args.resize(field_args.args.size());
                for (size_t i = 0; i < parent_args.size(); i++)
                    parent_args[i] = field_args.args[i]->eval();
                break;
            }
        }

        compiler->get_name_resolver().call_constructor(self_as_parent, std::move(parent_args));
    }

    auto& fields = class_type->fields();
    size_t new_fields_count = compiler->get_name_resolver().owned_fields_count[class_type->name];
    size_t parent_fields_count = fields.size() - new_fields_count;

    for (size_t i = 0; i < fields.size(); i++)
    {
        auto& field = fields[i];

        if (field.name.empty()) continue;  // A placeholder

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

        auto instance = compiler->create_value(ptr, type);

        if (i != 0 && i < parent_fields_count)
        {
            // Parent fields should only be set in case there is a constructor argument,
            //  otherwise, the parent constructor takes care of them.
            // There is an exception for the vtable, which is at index 0
            if (found) {
                name_resolver().call_constructor(instance, std::move(args));
            }
            continue;
        }

        // For a field with a name x:
        //  Constructor initializer args -> constructor() : x(a, b, c)
        //  initializer args -> X x(1, 2, 3)
        //  initializer -> X x = 3
        //  Type default value -> for most types, just a zero-initialization

        // For a field initialization, there are 4 cases:
        //  1 - It's a parent field. In this case, it won't have neither
        //   an initializer, nor an initializer arguments, but might have
        //   a constructor initializer arguments
        //  2 - It's a field of this class, with no initializers
        //  3 - It's a field of this class, with an initializer. In this case,
        //   it can't have an initializer args, because this will result in a
        //   parsing error
        //  4 - It's a field of this class, with an initializer args, and for
        //   the same reason as the above argument, it doesn't have an initializer
        // This means that if this is not a parent field, and there are no constructor
        //  initializer arguments, we can put both the initializer and the initializer
        //  arguments in one vector, and pass it to the constructor of the field

        if (args.empty())
        {
            if (field.initializer != nullptr)
                args.push_back(field.initializer->eval());
            for (auto& arg : field.init_args)
                args.push_back(arg->eval());
        }

        auto same_type = args.size() == 1 && args.front().type->get_concrete_type() == instance.type->get_concrete_type();
        if (same_type && args.front().is_teleporting) {
            // This is a special case
            builder().CreateStore(args.front().get(), instance.get());
        } else {
            name_resolver().call_constructor(instance, std::move(args));
        }
    }
}

void FunctionDefinitionNode::set_full_name()
{
    if (!nomangle) {
        name = name_resolver().get_function_full_name(name, get_function_type()->param_types);
        nomangle = true;
    }
}

FunctionDefinitionNode *FunctionDefinitionNode::clone() const {
    return compiler->create_node<FunctionDefinitionNode>(name, body, function_type, nomangle, template_param_count);
}

void FunctionDefinitionNode::destruct_fields(const ClassType* class_type)
{
    if (class_type->name == "Object") return;

    // Self is a reference. Turn it into a pointer
    auto self = name_resolver().symbol_table.get("self");
    self.type = compiler->create_type<PointerType>(self.type->as<ReferenceType>()->get_element_type());

    auto& fields = class_type->fields();
    size_t new_fields_count = compiler->get_name_resolver().owned_fields_count[class_type->name];
    size_t parent_fields_count = fields.size() - new_fields_count;

    // fields.size() will never be 0 since every object contains the vtable.
    for (size_t i = fields.size() - 1; i >= parent_fields_count; i--)
    {
        // Call the destructors of fields after calling the destructor of the class
        // Fields are destructed in the reverse order of definition.
        auto field = class_type->get_field(self, i);
        auto is_obj = dynamic_cast<const ClassType*>(field.type) != nullptr;
        if (!is_obj) continue;
        name_resolver().call_destructor(field);
    }

    // Here, we don't call the call_destructor method, instead, we call
    //  the destructor of the parent manually. This is because call_destructor
    //  loads the destructor from the vtable to choose the correct destructor
    //  for the instance dynamically. This is not the behaviour we want. Instead,
    //  we need to call the parent destructor statically with no dynamic dispatch.
    auto parent = compiler->get_name_resolver().parent_classes[class_type->name];
    auto parent_ptr = compiler->create_type<PointerType>(parent);
    auto self_as_parent = self.cast_as(parent_ptr);
    auto parent_ref = compiler->create_type<ReferenceType>(parent, false);
    self_as_parent.type = parent_ref;
    self_as_parent.memory_location = self.get();
    self_as_parent.set(nullptr);
    name_resolver().call_function(parent->name + ".destructor", { self_as_parent });
}

}
