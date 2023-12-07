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
    reset_symbol_table();
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
        std::vector<const Type*>{ compiler->create_type<PointerType>(cls) },
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

void ParserAssistant::reset_symbol_table() {
    compiler->name_resolver.symbol_table = decltype(compiler->name_resolver.symbol_table){};
}

std::vector<ASTNode *> ParserAssistant::pop_args()
{
    size_t n = leave_arg_list();
    std::vector<ASTNode*> args(n);
    for (size_t i = 0; i < n; i++)
        args[n - i - 1] = pop_node();
    return args;
}

void ParserAssistant::create_variable_declaration()
{
    auto name = pop_str();
    auto type = pop_type();
    compiler->name_resolver.symbol_table.insert(name, compiler->create_value(nullptr, type));
    if (is_in_global_scope()) {
        push_node<GlobalVariableDefinitionNode>(std::move(name), type, nullptr);
    } else {
        push_node<LocalVariableDefinitionNode>(std::move(name), type, nullptr);
    }
    inc_statements();
}

void ParserAssistant::create_variable_definition()
{
    // Either args or initializer
    auto args = pop_args();
    auto initializer = pop_node();
    auto name = pop_str();
    auto type = pop_type();

    // A temporary insertion into the symbol table for name resolution
    //  during parsing. The symbol table will be reset at the end.
    compiler->name_resolver.symbol_table.insert(name, compiler->create_value(nullptr, type));

    if (is_in_global_scope())
        push_node<GlobalVariableDefinitionNode>(std::move(name), type, initializer, std::move(args));
    else
        push_node<LocalVariableDefinitionNode> (std::move(name), type, initializer, std::move(args));

    inc_statements();
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

    // The params are pushed in reverse-order
    for (int i = 0; i < param_count; i++) {
        param_names[param_count - i - 1] = pop_str();
        param_types[param_count - i - 1] = pop_type();
    }

    auto return_type = pop_type();

    auto name = pop_str();
    if (compiler->current_class != nullptr) {
        name = compiler->current_class->getName().str() + '.' + name;
        auto class_name = compiler->current_class->getName().str();
        param_types.insert(
        param_types.begin(),
            compiler->create_type<PointerType>(
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
}

void ParserAssistant::create_function_definition_expression_body()
{
    ASTNode* body = compiler->create_node<ReturnNode>(pop_node());
    // The function is not popped, so it's still in the stack.
    auto function = pop_node_as<FunctionDefinitionNode>();
    function->set_body(body);
    nodes.push_back(function);
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
    compiler->name_resolver.push_scope();
}

size_t ParserAssistant::leave_scope() {
    size_t result = statement_counters.back();
    statement_counters.pop_back();
    compiler->name_resolver.pop_scope();
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
    auto lhs = pop_node_as<LValueNode>();
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
    auto instance = compiler->name_resolver.symbol_table.get(instance_name);
    auto class_type = dynamic_cast<const ClassType*>(instance.type);
    auto full_name = class_type->name + "." + func_name;
    auto args = pop_args();
    args.insert(args.begin(), compiler->create_node<VariableNode>(instance_name));
    push_node<FunctionCallNode>(full_name, std::move(args));
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
    push_node<CompoundAssignmentExpressionNode<AdditionNode>>(
        pop_node_as<LValueNode>(),
        compiler->create_node<I32ValueNode>(1)
    );
}

void ParserAssistant::create_pre_dec() {
    push_node<CompoundAssignmentExpressionNode<SubtractionNode>>(
        pop_node_as<LValueNode>(),
        compiler->create_node<I32ValueNode>(1)
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

    llvm::StructType::create(compiler->context, name);
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
    compiler->name_resolver.symbol_table.insert("self", compiler->create_value(nullptr, compiler->name_resolver.get_class(name)));
}

void ParserAssistant::create_class()
{
    size_t n = leave_scope();

    if (!is_in_global_scope())
        report_error("Class defined in a non-global scope");

    std::vector<ASTNode*> members(n);
    for (size_t i = 0; i < n; i++)
        members[n - i - 1] = pop_node();

    push_node<ClassDefinitionNode>(pop_str(), std::move(members), is_packed);

    compiler->current_class = nullptr;

    inc_statements();
}

void ParserAssistant::create_class_type() {
    push_type<ClassType>(pop_str());
}

void ParserAssistant::create_field_access() {
    push_node<ClassFieldNode>(pop_node(), pop_str());
}

void ParserAssistant::create_inferred_definition()
{
    const Type* type = nodes.back()->get_type();
    types.push_back(type);
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
    types.push_back(nodes.back()->get_type());
    pop_node();
}

void ParserAssistant::create_typename_type() {
    auto type = pop_type();
    auto llvm_type = type->llvm_type();
    if (llvm_type == nullptr) {
        // This can happen for example in the case typename(x).
        //  This will leave the parser confused about whether
        //  x is a variable name (an expression), or a class
        //  type. If the type is nullptr, this means that this
        //  is not a valid class type, thus, this is a variable.
        auto cls = dynamic_cast<const ClassType*>(type);
        if (cls == nullptr)
            report_internal_error("sizeof operator called on an invalid type");
        auto real_type = compiler->name_resolver.symbol_table.get(cls->name).type;
        push_node<StringValueNode>(real_type->to_string());
    } else {
        push_node<StringValueNode>(type->to_string());
    }
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

void ParserAssistant::create_malloc() {
    if (!declared_malloc)
    {
        auto type = compiler->create_type<FunctionType> (
            compiler->create_type<PointerType>(compiler->create_type<I64Type>()),
            std::vector<const Type*>{ compiler->create_type<I64Type>() }
        );

        compiler->name_resolver.register_function("malloc", {type, { "size" }}, true);

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

        compiler->name_resolver.register_function("free", {std::move(type), { "size" }}, true);

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

}
