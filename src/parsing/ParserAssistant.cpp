#include "parsing/ParserAssistant.hpp"


namespace dua
{

#define get_ixx(FUNC)                                         \
int insertion_point = 0;                                      \
for (int i = 0; i < num.size(); i++)                          \
    if (num[i] != '\'')                                       \
        num[insertion_point++] = num[i];                      \
num.resize(insertion_point);                                  \
                                                              \
if (num[0] == '0')                                            \
{                                                             \
    if (num.size() > 2)                                       \
    {                                                         \
        if (num[1] == 'x')                                    \
            return FUNC(num.c_str() + 2, nullptr, 16);        \
        else if (num[1] == 'b')                               \
            return FUNC(num.c_str() + 2, nullptr, 2);         \
    }                                                         \
    if (num.size() > 1)                                       \
        return FUNC(num.c_str() + 1, nullptr, 8);             \
}                                                             \
return FUNC(num);

int64_t ParserAssistant::get_i64(std::string num) { get_ixx(std::stoll); }
int32_t ParserAssistant::get_i32(std::string num) { get_ixx(std::stoi); }
int16_t ParserAssistant::get_i16(std::string num) { get_ixx(std::stoi); }
int8_t  ParserAssistant::get_i8 (std::string num) { get_ixx(std::stoi); }

TranslationUnitNode* ParserAssistant::construct_result()
{
    size_t n = nodes.size();
    std::vector<ASTNode*> elements(n);
    for (int i = 0; i < n; i++)
        elements[n - i - 1] = pop_node();
    return compiler->create_node<TranslationUnitNode>(std::move(elements));
}

void ParserAssistant::finish_parsing()
{
    compiler->current_class = nullptr;
    compiler->current_function = nullptr;
    create_missing_methods();
}

void ParserAssistant::create_missing_methods()
{
    for (auto& cls : compiler->name_resolver.classes) {
        create_empty_method_if_doesnt_exist(cls.second, "constructor");
        create_empty_method_if_doesnt_exist(cls.second, "destructor");
    }
}

void ParserAssistant::create_empty_method_if_doesnt_exist(const ClassType* cls, std::string&& name)
{
    name = cls->name + "." + name;

    if (compiler->name_resolver.has_function(name))
        return;

    auto type = compiler->create_type<FunctionType>(
        compiler->create_type<VoidType>(),
        std::vector<const Type*>{ compiler->create_type<ReferenceType>(cls) },
        false
    );

    auto signature = FunctionInfo {
        type,
        { "self" }
    };

    name = compiler->name_resolver.get_full_function_name(name, type->param_types);

    compiler->name_resolver.register_function(name, std::move(signature), true);

    compiler->push_deferred_node(
        compiler->create_node<FunctionDefinitionNode>(
            name,
            compiler->create_node<BlockNode>(std::vector<ASTNode*>{}),
            type
        )
    );
}

std::vector<ASTNode *> ParserAssistant::pop_args()
{
    size_t n = leave_arg_list();
    std::vector<ASTNode*> args(n);
    for (size_t i = 0; i < n; i++)
        args[n - i - 1] = pop_node();
    return args;
}

std::vector<const Type*> ParserAssistant::pop_types()
{
    size_t n = leave_arg_list();
    std::vector<const Type*> types(n);
    for (size_t i = 0; i < n; i++)
        types[n - i - 1] = pop_type();
    return types;
}

void ParserAssistant::create_variable_declaration()
{
    auto name = pop_str();
    auto type = pop_type();

    if (is_in_global_scope()) {
        push_node<GlobalVariableDefinitionNode>(std::move(name), type, nullptr);
    } else {
        if (compiler->current_function != nullptr) {
            push_node<LocalVariableDefinitionNode>(std::move(name), type, nullptr);
        } else {
            // This is a field
            assert(compiler->current_class != nullptr);
            auto class_name = compiler->current_class->getName().str();
            compiler->name_resolver.class_fields[class_name].push_back({ name, type, nullptr, {} });
            push_node<ClassFieldDefinitionNode>(std::move(name), type, nullptr);
        }
    }

    inc_statements();
}

void ParserAssistant::create_variable_definition()
{
    // Either args or initializer
    auto args = pop_args();
    auto initializer = pop_node();

    create_variable_declaration();

    auto node = pop_node_as<VariableDefinitionNode>();
    node->initializer = initializer;
    node->args = std::move(args);
    nodes.push_back(node);
}

void ParserAssistant::create_function_declaration()
{
    if (!is_in_global_scope() && compiler->current_class == nullptr)
        report_error("Function declarations/definitions not allowed in a local scope");

    inc_statements();

    auto is_var_arg = pop_var_arg();
    auto param_count = leave_arg_list();
    std::vector<const Type*> param_types(param_count);
    std::vector<std::string> param_names(param_count);

    for (int i = 0; i < param_count; i++) {
        param_names[param_count - i - 1] = pop_str();
        param_types[param_count - i - 1] = pop_type();
    }

    auto return_type = pop_type();

    auto name = pop_str();
    if (compiler->current_class != nullptr) {
        auto class_name = compiler->current_class->getName().str();
        name = class_name + '.' + name;
        param_types.insert(
        param_types.begin(),
            compiler->create_type<ReferenceType>(
                compiler->name_resolver.classes[class_name]
            )
        );
        param_names.insert(param_names.begin(), "self");
    }

    if (!no_mangle)
        name = compiler->name_resolver.get_full_function_name(name, param_types);
    else if (compiler->current_class != nullptr)
        report_error("Can't use the no_mangle keyword for methods");

    auto function_type = compiler->create_type<FunctionType>(return_type, std::move(param_types), is_var_arg);
    FunctionInfo info {
        function_type,
        std::move(param_names)
    };

    compiler->name_resolver.register_function(name, std::move(info), true);

    push_node<FunctionDefinitionNode>(std::move(name), nullptr, function_type);
}

void ParserAssistant::create_block()
{
    size_t n = leave_scope();
    std::vector<ASTNode*> statements(n);
    for (int i = 0; i < n; i++)
        statements[n - i - 1] = pop_node();
    push_node<BlockNode>(statements);
}

void ParserAssistant::create_function_definition_block_body()
{
    ASTNode* body = pop_node();
    // The function is not popped, so it's still in the stack.
    auto function = pop_node_as<FunctionDefinitionNode>();
    function->set_body(body);
    nodes.push_back(function);
    dec_statements();
    compiler->current_function = nullptr;
}

void ParserAssistant::create_function_definition_expression_body()
{
    ASTNode* body = compiler->create_node<ReturnNode>(pop_node());
    // The function is not popped, so it's still in the stack.
    auto function = pop_node_as<FunctionDefinitionNode>();
    function->set_body(body);
    nodes.push_back(function);
    compiler->current_function = nullptr;
}

void ParserAssistant::create_if_statement()
{
    bool create_else = has_else.back();
    size_t n = leave_conditional();

    std::vector<ASTNode*> conditions(n);
    std::vector<ASTNode*> branches(n);

    ASTNode* else_node = nullptr;
    if (create_else) {
        else_node = pop_node();
        dec_statements();
    }

    for (size_t i = 0; i < n; i++) {
        branches[n - i - 1] = pop_node();
        conditions[n - i - 1] = pop_node();
        dec_statements();
    }

    if (else_node) branches.push_back(else_node);

    push_node<IfNode>(std::move(conditions), std::move(branches), false);

    inc_statements();
}

void ParserAssistant::create_if_expression()
{
    bool create_else = has_else.back();
    size_t n = leave_conditional();

    std::vector<ASTNode*> conditions(n);
    std::vector<ASTNode*> branches(n);

    ASTNode* else_node = nullptr;
    if (create_else) {
        else_node = pop_node();
    }

    for (size_t i = 0; i < n; i++) {
        branches[n - i - 1] = pop_node();
        conditions[n - i - 1] = pop_node();
    }

    if (else_node) branches.push_back(else_node);

    push_node<IfNode>(std::move(conditions), std::move(branches), true, pop_str());
}


void ParserAssistant::enter_scope() {
    statement_counters.push_back(0);
    compiler->push_scope();
}

size_t ParserAssistant::leave_scope() {
    size_t result = statement_counters.back();
    statement_counters.pop_back();
    compiler->pop_scope();
    return result;
}

void ParserAssistant::enter_conditional() {
    branch_counters.push_back(0);
    has_else.push_back(false);
}

size_t ParserAssistant::leave_conditional() {
    size_t result = branch_counters.back();
    branch_counters.pop_back();
    has_else.pop_back();
    return result;
}

void ParserAssistant::inc_statements() {
    statement_counters.back() += 1;
}

void ParserAssistant::dec_statements() {
    statement_counters.back() -= 1;
}

void ParserAssistant::inc_branches() {
    branch_counters.back() += 1;
}

void ParserAssistant::set_has_else() {
    has_else.back() = true;
}

bool ParserAssistant::is_in_global_scope() {
    return statement_counters.size() == 1;
}

void ParserAssistant::create_expression_statement() {
    push_node<ExpressionStatementNode>(pop_node());
    // It takes an expression that didn't count as a
    //  statement, and turn it into a statement.
    inc_statements();
}

void ParserAssistant::create_return() {
    push_node<ReturnNode>(pop_node());
    // It takes an expression that didn't count as a
    //  statement, and turn it into a statement.
    inc_statements();
}

void ParserAssistant::create_while()
{
    ASTNode* body = pop_node();
    ASTNode* condition = pop_node();
    push_node<WhileNode>(condition, body);
}

void ParserAssistant::create_assignment()
{
    // We don't increment the statements counter here since this
    //  is an expression, and the expression statement would increase
    //  it as appropriate
    ASTNode* rhs = pop_node();
    ASTNode* lhs = pop_node();
    push_node<AssignmentExpressionNode>(lhs, rhs);
}

void ParserAssistant::enter_arg_list() {
    argument_counters.push_back(0);
}

size_t ParserAssistant::leave_arg_list() {
    size_t result = argument_counters.back();
    argument_counters.pop_back();
    return result;
}

void ParserAssistant::inc_args() {
    argument_counters.back() += 1;
}

void ParserAssistant::push_var_arg(bool value) {
    var_arg_stack.push_back(value);
}

bool ParserAssistant::pop_var_arg() {
    bool result = var_arg_stack.back();
    var_arg_stack.pop_back();
    return result;
}

void ParserAssistant::create_function_call() {
    push_node<FunctionCallNode>(pop_str(), pop_args());
}

void ParserAssistant::create_method_call()
{
    auto func_name = pop_str();
    auto instance_name = pop_str();
    auto args = pop_args();
    push_node<MethodCallNode>(std::move(instance_name), std::move(func_name), std::move(args));
}

void ParserAssistant::create_expr_function_call()
{
    push_node<ExprFunctionCallNode>(pop_node(), pop_args());
}

void ParserAssistant::create_cast()
{
    auto type = pop_type();
    auto expr = pop_node();
    push_node<CastExpressionNode>(expr, type);
}

void ParserAssistant::create_pointer_type() {
    push_type<PointerType>(pop_type());
}

void ParserAssistant::create_array_type() {
    auto type = pop_type();
    auto size = pop_num();
    push_type<ArrayType>(type, size);
}

void ParserAssistant::create_string_value()
{
    auto str = pop_str();
    push_node<StringValueNode>(escape_characters(
        str.substr(1, str.size() - 2)
    ));
}

void ParserAssistant::create_dereference() {
    push_node<DereferenceNode>(pop_node());
}

void ParserAssistant::create_pre_inc() {
    auto lvalue = pop_node_as<LValueNode>();
    push_node<PrefixAdditionExpressionNode>(
        lvalue,
        1
    );
}

void ParserAssistant::create_pre_dec() {
    auto lvalue = pop_node_as<LValueNode>();
    push_node<PrefixAdditionExpressionNode>(
        lvalue,
        -1
    );
}

void ParserAssistant::create_post_inc() {
    push_node<PostfixAdditionExpressionNode>(pop_node_as<LValueNode>(), 1);
}

void ParserAssistant::create_post_dec() {
    push_node<PostfixAdditionExpressionNode>(pop_node_as<LValueNode>(), -1);
}

void ParserAssistant::create_ternary_operator() {
    enter_conditional();
    inc_branches();
    set_has_else();
    create_if_expression();
}

void ParserAssistant::create_for()
{

    auto update = pop_node();
    auto body = pop_node();
    auto condition = pop_node();

    // Decrement for popping the body statement
    dec_statements();

    auto n = leave_scope();
    std::vector<ASTNode*> initializations(n);
    for (int i = 0; i < n; i++)
        initializations[n - i - 1] = pop_node();

    inc_statements();

    push_node<ForNode>(std::move(initializations), condition, update, body);
}

void ParserAssistant::create_empty_statement() {
    push_node<I8ValueNode>(0);
    inc_statements();
}

void ParserAssistant::create_continue() {
    push_node<ContinueNode>();
    inc_statements();
}

void ParserAssistant::create_break() {
    push_node<BreakNode>();
    inc_statements();
}

void ParserAssistant::create_do_while() {
    ASTNode* condition = pop_node();
    ASTNode* body = pop_node();
    push_node<DoWhileNode>(condition, body);
}

void ParserAssistant::create_loaded_lvalue() {
    auto node = pop_node_as<LValueNode>();
    push_node<LoadedLValueNode>(node);
}

void ParserAssistant::create_array_indexing() {
    auto index = pop_node();
    auto arr = pop_node_as<LValueNode>();
    push_node<ArrayIndexingNode>(arr, index);
}

void ParserAssistant::create_logical_and()
{
    auto rhs = pop_node();
    auto lhs = pop_node();

    auto zero = compiler->create_node<I32ValueNode>(0);
    auto one = compiler->create_node<I32ValueNode>(1);

    auto rhs_casted = compiler->create_node<NENode>(rhs, zero);
    std::vector<ASTNode*> inner_conditions { rhs_casted };
    std::vector<ASTNode*> inner_branches { one, zero };
    auto inner = compiler->create_node<IfNode>(std::move(inner_conditions),
                                               std::move(inner_branches), true, "And");

    auto lhs_casted = compiler->create_node<NENode>(lhs, zero);
    std::vector<ASTNode*> outer_conditions { lhs_casted };
    std::vector<ASTNode*> outer_branches { inner, zero };
    auto outer = compiler->create_node<IfNode>(std::move(outer_conditions),
                                               std::move(outer_branches), true, "And");

    nodes.push_back(outer);
}

void ParserAssistant::create_logical_or()
{
    auto rhs = pop_node();
    auto lhs = pop_node();

    auto zero = compiler->create_node<I32ValueNode>(0);
    auto one = compiler->create_node<I32ValueNode>(1);

    auto rhs_casted = compiler->create_node<NENode>(rhs, zero);
    std::vector<ASTNode*> inner_conditions { rhs_casted };
    std::vector<ASTNode*> inner_branches { one, zero };
    auto inner = compiler->create_node<IfNode>(std::move(inner_conditions),
                           std::move(inner_branches), true, "Or");

    auto lhs_casted = compiler->create_node<NENode>(lhs, zero);
    std::vector<ASTNode*> outer_conditions { lhs_casted };
    std::vector<ASTNode*> outer_branches { one, inner };
    auto outer = compiler->create_node<IfNode>(std::move(outer_conditions),
                           std::move(outer_branches), true, "Or");

    nodes.push_back(outer);
}

void ParserAssistant::register_class()
{
    auto& name = strings.back();

    if (compiler->name_resolver.classes.find(name) != compiler->name_resolver.classes.end()) {
        // No need to register it again
        return;
    }

    auto cls = compiler->create_type<ClassType>(name);
    compiler->name_resolver.classes[name] = cls;
}

void ParserAssistant::finish_class_declaration() {
    // Pop the name since it's not going to be used in a definition
    pop_str();
}

void ParserAssistant::start_class_definition()
{
    if (compiler->current_class)
        report_error("Nested classes are not allowed");

    if (compiler->current_function)
        report_error("Can't define a class inside a function");


    auto& name = strings.back();
    compiler->current_class = compiler->name_resolver.get_class(name)->llvm_type();

    if (compiler->current_class->isSized())
        report_error("Redefinition of the class " + name);
}

void ParserAssistant::create_class()
{
    size_t n = leave_scope();

    if (!is_in_global_scope())
        report_error("Class defined in a non-global scope");

    std::vector<ClassFieldDefinitionNode*> fields;
    std::vector<FunctionDefinitionNode*> methods;
    std::vector<TypeAliasNode*> aliases;

    for (size_t i = 0; i < n; i++) {
        auto node = pop_node();
        if (auto f = dynamic_cast<ClassFieldDefinitionNode*>(node); f != nullptr)
            fields.push_back(f);
        else if (auto m = dynamic_cast<FunctionDefinitionNode*>(node); m != nullptr)
            methods.push_back(m);
        else if (auto a = dynamic_cast<TypeAliasNode*>(node); a != nullptr)
            aliases.push_back(a);
        else report_internal_error("Class member that's not a field, a method, or an alias");
    }

    // So that they are in the order of definition.
    std::reverse(fields.begin(), fields.end());

    std::vector<llvm::Type*> body(fields.size());
    for (size_t i = 0; i < fields.size(); i++)
        body[i] = fields[i]->get_type()->llvm_type();

    auto name = pop_str();

    assert(compiler->current_class == compiler->name_resolver.get_class(name)->llvm_type());
    compiler->current_class->setBody(std::move(body), is_packed);

    push_node<ClassDefinitionNode>(std::move(name), std::move(fields), std::move(methods), std::move(aliases));

    compiler->current_class = nullptr;

    inc_statements();
}

void ParserAssistant::create_identifier_type() {
    push_type<IdentifierType>(pop_str());
}

void ParserAssistant::create_field_access() {
    push_node<ClassFieldNode>(pop_node(), pop_str());
}

void ParserAssistant::create_inferred_definition()
{
    push_type<TypeOfType>(nodes.back());
    create_variable_definition();
}

void ParserAssistant::create_size_of_type()
{
    push_node<SizeOfNode>(pop_type());
}

void ParserAssistant::create_size_of_expression()
{
    create_type_of();
    create_size_of_type();
}

void ParserAssistant::create_type_of()
{
    push_type<TypeOfType>(pop_node());
}

void ParserAssistant::create_typename_type()
{
    push_node<TypeNameNode>(pop_type());
}

void ParserAssistant::create_typename_expression() {
    create_type_of();
    create_typename_type();
}

void ParserAssistant::create_function_type()
{
    size_t param_count = leave_arg_list();
    bool is_var_arg = pop_var_arg();
    std::vector<const Type*> param_types(param_count);
    for (size_t i = 0; i < param_count; i++)
        param_types[param_count - i - 1] = pop_type();
    const Type* return_type = pop_type();
    push_type<FunctionType>(return_type, std::move(param_types), is_var_arg);
}

void ParserAssistant::create_malloc()
{
    if (!declared_malloc)
    {
        auto type = compiler->create_type<FunctionType> (
            compiler->create_type<PointerType>(compiler->create_type<I64Type>()),
            std::vector<const Type*>{ compiler->create_type<I64Type>() }
        );

        // Temporarily setting the current function to nullptr to avoid the nested function error
        auto current_function = compiler->current_function;
        compiler->current_function = nullptr;
        compiler->name_resolver.register_function("malloc", {type, { "size" }}, true);
        compiler->current_function = current_function;

        declared_malloc = true;
    }
    push_node<MallocNode>(pop_type(), pop_args());
}

void ParserAssistant::create_free()
{
    if (!declared_free)
    {
        auto type = compiler->create_type<FunctionType> (
            compiler->create_type<VoidType>(),
            std::vector<const Type*>{ compiler->create_type<PointerType>(compiler->create_type<I64Type>()) }
        );

        // Temporarily setting the current function to nullptr to avoid the nested function error
        auto current_function = compiler->current_function;
        compiler->current_function = nullptr;
        compiler->name_resolver.register_function("free", {std::move(type), { "size" }}, true);
        compiler->current_function = current_function;

        declared_free = true;
    }
    push_node<FreeNode>(pop_node());
    inc_statements();
}

void ParserAssistant::create_pointer_field_access() {
    push_node<ClassFieldNode>(pop_node(), pop_str());
}

void ParserAssistant::prepare_constructor()
{
    if (compiler->current_class == nullptr)
        report_error("Constructors can only be defined inside classes");
    push_type<VoidType>();
    push_str("constructor");
    no_mangle = false;
}

void ParserAssistant::finish_constructor()
{
    auto constructor = pop_node_as<FunctionDefinitionNode>();
    compiler->name_resolver.add_fields_constructor_args(constructor->name, std::move(fields_args));
    nodes.push_back(constructor);
}

void ParserAssistant::prepare_destructor()
{
    if (compiler->current_class == nullptr)
        report_error("Destructors can only be defined inside classes");
    push_type<VoidType>();
    push_str("destructor");
    no_mangle = false;
}

void ParserAssistant::add_field_constructor_args() {
    fields_args.push_back({ pop_str(), pop_args() });
}

void ParserAssistant::create_array_literal() {
    push_node<ArrayValueNode>(pop_args(), pop_type());
}

void ParserAssistant::create_forced_cast() {
    push_node<CastExpressionNode>(pop_node(), pop_type(), true);
}

void ParserAssistant::create_func_ref()
{
    auto param_types = pop_types();
    auto ret_type = pop_type();
    auto target_func = pop_str();
    auto is_var_arg = pop_var_arg();
    auto func_type = compiler->create_type<FunctionType>(ret_type, param_types, is_var_arg);
    push_type<PointerType>(func_type);
    push_node<VariableNode>(target_func, types.back());

    enter_arg_list();
    create_variable_definition();
}

void ParserAssistant::prepare_copy_constructor()
{
    if (compiler->current_class == nullptr)
        report_error("Copy constructors can only be defined inside classes");
    push_type<VoidType>();
    push_str("=constructor");
    no_mangle = false;
}

void ParserAssistant::finish_copy_constructor()
{
    auto constructor = pop_node_as<FunctionDefinitionNode>();
    auto& param_types = constructor->get_function_type()->param_types;
    if (param_types.size() != 2)  // One for the instance and one for the other object
        report_error("Copy constructors must have exactly one parameter");
    compiler->name_resolver.add_fields_constructor_args(constructor->name, std::move(fields_args));
    nodes.push_back(constructor);
}

void ParserAssistant::create_infix_operator()
{
    auto param_count = leave_arg_list();
    std::vector<const Type*> param_types(param_count);
    std::vector<std::string> param_names(param_count);

    inc_statements();

    for (int i = 0; i < param_count; i++) {
        param_names[param_count - i - 1] = pop_str();
        param_types[param_count - i - 1] = pop_type();
    }

    auto name = pop_str();

    bool is_method = compiler->current_class != nullptr;
    if (param_count + is_method != 2) {
        if (is_method) {
            auto class_name = compiler->current_class->getName().str();
            report_error("Class infix operators expect exactly one parameter (in " + class_name + "::" + name + " operator)");
        } else {
            report_error("Global infix operators expect exactly two parameters (in the global " + name + " operator)");
        }
    }

    auto return_type = pop_type();

    if (compiler->current_class != nullptr) {
        auto class_name = compiler->current_class->getName().str();
        param_types.insert(
            param_types.begin(),
            compiler->create_type<ReferenceType>(
                compiler->name_resolver.classes[class_name]
            )
        );
        param_names.insert(param_names.begin(), "self");
    }

    name = "infix." + name + "." + param_types[0]->to_string() + "." + param_types[1]->to_string();

    auto function_type = compiler->create_type<FunctionType>(return_type, std::move(param_types), false);
    FunctionInfo info {
        function_type,
        std::move(param_names)
    };

    compiler->name_resolver.register_function(name, std::move(info), true);

    push_node<FunctionDefinitionNode>(std::move(name), nullptr, function_type);
}

void ParserAssistant::set_current_function()
{
    auto func = pop_node_as<FunctionDefinitionNode>();
    compiler->current_function = compiler->module.getFunction(func->name);
    nodes.push_back(func);
}

void ParserAssistant::create_reference_type() {
    push_type<ReferenceType>(pop_type());
}

void ParserAssistant::create_type_alias() {
    auto name = pop_str();
    auto type = pop_type();
    push_node<TypeAliasNode>(std::move(name), type);
    inc_statements();
}

}
