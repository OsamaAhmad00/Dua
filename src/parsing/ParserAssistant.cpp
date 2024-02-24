#include <queue>
#include "parsing/ParserAssistant.hpp"
#include "resolution/TemplatedNameResolver.hpp"


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
int32_t ParserAssistant::get_i32(std::string num) { get_ixx(std::stoll); }
int16_t ParserAssistant::get_i16(std::string num) { get_ixx(std::stoll); }
int8_t  ParserAssistant::get_i8 (std::string num) { get_ixx(std::stoll); }

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
    // By this point, all classes are declared. Functions must be declared before
    //  class definitions because class members may use the typeof operator on a
    //  function name or something similar

    if (!used_dynamic_casting)
        compiler->delete_dynamic_casting_function();

    for (auto alias : global_aliases)
        alias->eval();

    for (auto& [constructor, args, owner_class, in_templated_class] : constructors_field_args)
    {
        if (in_templated_class)
        {
            compiler->name_resolver.templated_class_field_constructor_args[owner_class].push_back({ constructor, std::move(args) });
        }
        else
        {
            if (!owner_class.empty()) {
                compiler->typing_system.push_scope();
                auto& aliases = class_info[owner_class].node->aliases;
                for (auto& alias : aliases)
                    alias->eval();
            }

            constructor->set_full_name();
            compiler->name_resolver.add_fields_constructor_args(constructor->name, std::move(args));

            if (!owner_class.empty())
                compiler->typing_system.pop_scope();
        }
    }

    for (auto& [func, info, owner_class] : function_definitions)
    {
        if (!owner_class.empty()) {
            compiler->typing_system.push_scope();
            auto& aliases = class_info[owner_class].node->aliases;
            for (auto& alias : aliases)
                alias->eval();
        }

        func->set_full_name();
        compiler->name_resolver.register_function(func->name, std::move(info), true);

        if (!owner_class.empty())
            compiler->typing_system.pop_scope();
    }

    std::vector<llvm::Type*> body;

    for (auto it = templated_class_definitions.begin(); it != templated_class_definitions.end(); ) {
        auto& [name, template_args] = *it;
        it++;
        auto key = compiler->name_resolver.get_templated_class_key(name, template_args.size());
        if (!compiler->name_resolver.has_templated_class(key)) {
            // This is a templated function reference that got here by mistake
            it = templated_class_definitions.erase(--it);
            continue;
        }
        for (auto& arg : template_args) {
            if (!arg->is_resolvable_now()) {
                // This type will be resolved recursively somewhere else
                // An example of this is a templated parent class with a template
                //  argument that is a template parameter of the child.
                it = templated_class_definitions.erase(--it);
                break;
            }
        }
    }

    for (auto& [name, template_args] : templated_class_definitions) {
        auto full_name = compiler->name_resolver.get_templated_class_full_name(name, template_args);
        if (!compiler->name_resolver.has_class(full_name))
            compiler->name_resolver.register_templated_class(name, template_args);
    }

    for (auto& [name, template_args] : templated_class_definitions) {
        auto cls = compiler->name_resolver.get_templated_class(name, template_args);
        auto& info = class_info[cls->name];  // Using the full name to avoid collisions
        if (!info.name.empty())
            compiler->report_error("Redefinition of the templated class " + cls->name + " with " + std::to_string(template_args.size()) + " template parameters");
        info.template_args = template_args;
        info.is_templated = true;
        info.name = name;
    }

    for (auto& [name, node] : class_info) {
        // Templated nodes have already registered their parents
        if (!node.is_templated) {
            auto parent = compiler->name_resolver.get_parent_class(node.parent);
            compiler->name_resolver.parent_classes[node.name] = parent;
        }
    }

    // Here, after all templated and non-templated classes are registered,
    //  the name resolver is populated with the concrete parents of classes
    for (auto& [child_name, parent] : compiler->name_resolver.parent_classes) {
        class_info[parent->name].children.push_back(child_name);
    }

    // The missing methods are registered right after all classes (including templated ones)
    //  are registered, and before the classes are defined, so that these missing methods
    //  are visible to the class definitions.
    create_missing_methods();

    // Evaluate all global variable definitions before defining functions and classes
    for (auto node : global_variable_nodes)
        node->eval();

    for (auto& root : class_info["Object"].children)
    {
        if (class_info.find(root) == class_info.end())
            continue;

        std::queue<ClassInfo*> queue;
        queue.push(&class_info[root]);

        while (!queue.empty()) {
            auto node = queue.front();
            queue.pop();

            for (auto &child: node->children)
                queue.push(&class_info[child]);

            // The class might be defined earlier in the
            //  definition of one of the templated classes.
            //  If it's already defined, don't construct
            //  its fields again
            if (node->is_templated) {
                auto full_name = compiler->name_resolver.get_templated_class_full_name(node->name, node->template_args);
                if (compiler->name_resolver.class_fields[full_name].empty())
                    compiler->name_resolver.construct_templated_class_fields(node->name, node->template_args);
            } else {
                compiler->name_resolver.construct_class_fields(node->name, node->node);
            }
        }
    }

    for (auto& root : class_info["Object"].children)
    {
        if (class_info.find(root) == class_info.end())
            continue;

        std::queue<ClassInfo*> queue;
        queue.push(&class_info[root]);

        while (!queue.empty())
        {
            auto node = queue.front();
            queue.pop();

            for (auto& child : node->children)
                queue.push(&class_info[child]);

            if (node->is_templated) {
                if (!compiler->name_resolver.is_templated_class_defined(node->name, node->template_args)) {
                    compiler->name_resolver.define_templated_class(node->name, node->template_args);
                }
            } else {
                node->node->eval();
            }
        }
    }

    for (auto& [full_name, info] : compiler->name_resolver.registered_templated_classes) {
        if (!info.are_fields_constructed)
            compiler->name_resolver.construct_templated_class_fields(info.name, info.template_args);
        if (!info.is_defined)
            compiler->name_resolver.define_templated_class(info.name, info.template_args);
    }

    // Only the global scope
    assert(compiler->typing_system.identifier_types.size() == 1);

    current_class = "";
    is_in_function = false;
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
        std::vector<const Type*>{ compiler->create_type<ReferenceType>(cls, true) },
        false
    );

    auto info = FunctionInfo {
        type,
        { "self" },
        false,
        cls
    };

    name = compiler->name_resolver.get_function_full_name(name, type->param_types);

    compiler->name_resolver.register_function(name, std::move(info), true);

    compiler->push_deferred_node(
        compiler->create_node<FunctionDefinitionNode>(
            std::move(name),
            compiler->create_node<BlockNode>(std::vector<ASTNode*>{}),
            type,
            true
        )
    );
}

std::vector<std::string> ParserAssistant::pop_strings()
{
    size_t n = pop_counter();
    std::vector<std::string> result(n);
    for (size_t i = 0; i < n; i++)
        result[n - i - 1] = pop_str();
    return result;
}

std::vector<ASTNode *> ParserAssistant::pop_args()
{
    size_t n = pop_counter();
    std::vector<ASTNode*> args(n);
    for (size_t i = 0; i < n; i++)
        args[n - i - 1] = pop_node();
    return args;
}

std::vector<const Type*> ParserAssistant::pop_types()
{
    size_t n = pop_counter();
    std::vector<const Type*> types(n);
    for (size_t i = 0; i < n; i++)
        types[n - i - 1] = pop_type();
    return types;
}

void ParserAssistant::create_variable_declaration()
{
    auto name = pop_str();
    auto type = pop_type();

    if (is_in_global_scope())
    {
        push_node<GlobalVariableDefinitionNode>(std::move(name), type, nullptr, std::vector<ASTNode*>{}, is_extern, is_static);
        global_variable_nodes.push_back((GlobalVariableDefinitionNode*)nodes.back());
        is_extern = false;
        is_static = false;
    }
    else
    {
        if (is_extern) {
            auto identifier = type->as<IdentifierType>();
            compiler->report_error("The extern keyword can only be used with global variable "
                         "declarations (in the declaration of the variable " + name + " with type " +
                               (identifier != nullptr ? identifier->name : type->to_string()) + ")");
        }

        if (is_in_function) {
            push_node<LocalVariableDefinitionNode>(std::move(name), type);
        } else {
            // This is a field
            assert(in_class());
            compiler->name_resolver.class_fields[current_class].push_back({ name, type, nullptr, {} });
            push_node<ClassFieldDefinitionNode>(std::move(name), type);
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
    if (!is_in_global_scope() && !in_class())
        compiler->report_error("Function declarations/definitions not allowed in a local scope");

    inc_statements();

    auto is_var_arg = pop_var_arg();
    auto param_count = pop_counter();
    std::vector<const Type*> param_types(param_count);
    std::vector<std::string> param_names(param_count);

    for (int i = 0; i < param_count; i++) {
        param_names[param_count - i - 1] = pop_str();
        param_types[param_count - i - 1] = pop_type();
    }

    std::vector<std::string> template_params = pop_strings();

    auto name = pop_str();

    auto return_type = pop_type();

    bool is_templated = pop_is_templated();

    if (in_class())
    {
        if (!in_templated_class)
            name = current_class + '.' + name;
        // This i64 type is just a placeholder
        auto self_type = in_templated_class ? (const Type*)compiler->create_type<I64Type>()
                : compiler->create_type<ReferenceType>(compiler->name_resolver.classes[current_class], true);
        param_types.insert(param_types.begin(), self_type);
        param_names.insert(param_names.begin(), "self");
    }

    if (nomangle) {
        if (in_class())
            compiler->report_error("Can't use the nomangle keyword for methods");
        if (is_templated)
            compiler->report_error("Non-mangled templated functions are not supported yet");
    }

    auto function_type = compiler->create_type<FunctionType>(return_type, std::move(param_types), is_var_arg);
    FunctionInfo info {
        function_type,
        std::move(param_names),
        is_templated,
        in_templated_class ? nullptr : compiler->create_type<ClassType>(current_class)
    };

    size_t template_param_count = FunctionDefinitionNode::NOT_TEMPLATED;
    if (is_templated)
        template_param_count = template_params.size();

    push_node<FunctionDefinitionNode>(std::move(name), nullptr, function_type, nomangle, template_param_count, false, is_static);
    is_static = false;
    auto func = (FunctionDefinitionNode*)nodes.back();

    if (in_templated_class) {
        // Templated classes would add their own templated methods upon instantiation of concrete classes
        compiler->name_resolver.add_templated_class_method_info(current_class, func, std::move(info), std::move(template_params));
    } else if (is_templated) {
        // Templated functions will be registered upon instantiation
        compiler->name_resolver.add_templated_function(func, std::move(template_params), std::move(info), current_class, in_templated_class);
    } else {
        function_definitions.push_back({ func, std::move(info), current_class });
    }
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
    is_in_function = false;
}

void ParserAssistant::create_function_definition_expression_body()
{
    ASTNode* body = compiler->create_node<ReturnNode>(pop_node());
    // The function is not popped, so it's still in the stack.
    auto function = pop_node_as<FunctionDefinitionNode>();
    function->set_body(body);
    nodes.push_back(function);
    is_in_function = false;
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

void ParserAssistant::push_counter() {
    general_counters.push_back(0);
}

size_t ParserAssistant::pop_counter() {
    size_t result = general_counters.back();
    general_counters.pop_back();
    return result;
}

void ParserAssistant::inc_counter() {
    general_counters.back() += 1;
}

void ParserAssistant::push_var_arg(bool value) {
    var_arg_stack.push_back(value);
}

bool ParserAssistant::pop_var_arg() {
    bool result = var_arg_stack.back();
    var_arg_stack.pop_back();
    return result;
}

void ParserAssistant::create_function_call()
{
    auto args = pop_args();
    auto template_args = pop_types();
    auto func_name = pop_resolution_string();
    if (pop_is_templated())
        push_node<FunctionCallNode>(func_name, std::move(args), std::move(template_args));
    else
        push_node<FunctionCallNode>(func_name, std::move(args));
}

void ParserAssistant::create_method_call()
{
    auto args = pop_args();
    auto template_args = pop_types();
    auto func_name = pop_resolution_string();
    auto instance = pop_node();
    if (pop_is_templated())
        push_node<MethodCallNode>(instance, func_name, std::move(args), std::move(template_args));
    else
        push_node<MethodCallNode>(instance, func_name, std::move(args));
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
    push_node<PrefixAdditionExpressionNode>(pop_node(), 1);
}

void ParserAssistant::create_pre_dec() {
    push_node<PrefixAdditionExpressionNode>(pop_node(), -1);
}

void ParserAssistant::create_post_inc() {
    push_node<PostfixAdditionExpressionNode>(pop_node(), 1);
}

void ParserAssistant::create_post_dec() {
    push_node<PostfixAdditionExpressionNode>(pop_node(), -1);
}

void ParserAssistant::create_ternary_operator() {
    enter_conditional();
    inc_branches();
    set_has_else();
    push_str("ternary");
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

void ParserAssistant::create_indexing() {
    auto rhs = pop_node();
    auto lhs = pop_node();
    push_node<IndexingNode>(lhs, rhs);
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
    if (is_templated_stack.back())
        return;

    auto& name = strings.back().as_str();

    if (compiler->name_resolver.classes.find(name) != compiler->name_resolver.classes.end()) {
        // No need to register it again
        return;
    }

    if (compiler->name_resolver.has_function(name))
        compiler->report_error("There is already a function with the name " + name + ". Can't have a class with the same name");

    auto cls = compiler->create_type<ClassType>(name);
    compiler->name_resolver.classes[name] = cls;
    compiler->typing_system.insert_global_type(name, cls);
}

void ParserAssistant::finish_class_declaration() {
    // Pop the name since it's not going to be used in a definition
    pop_str();
    pop_strings();  // template params
    pop_is_templated();
}

void ParserAssistant::start_class_definition()
{
    if (in_class())
        compiler->report_error("Nested classes are not allowed");

    if (is_in_function)
        compiler->report_error("Can't define a class inside a function");

    in_templated_class = is_templated_stack.back();

    if (in_templated_class) {
        auto template_param_count = general_counters.back();
        auto& name = strings[strings.size() - 1 - template_param_count].as_str();
        current_class = compiler->name_resolver.get_templated_class_key(name, template_param_count);
    } else {
        current_class = strings.back().as_str();
    }
}

void ParserAssistant::create_class()
{
    // Pop the statement counter here so that checking global scope succeeds
    size_t n = leave_scope();

    if (!is_in_global_scope())
        compiler->report_error("Class " + current_class + " is defined in a non-global scope");

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
        else compiler->report_internal_error("Class member that's not a field, a method, or an alias");
    }

    // So that they are in the order of definition.
    std::reverse(fields.begin(), fields.end());

    auto template_params = pop_strings();
    auto name = pop_str();
    auto is_templated = pop_is_templated();

    for (size_t i = 0; i < fields.size(); i++)
        for (size_t j = i + 1; j < fields.size(); j++)
            if (fields[i]->name == fields[j]->name)
                compiler->report_error("The field " + fields[i]->name + " of the class " + name + " is defined more than once");

    auto parent_type = pop_type()->as<IdentifierType>();

    auto cls = compiler->create_node<ClassDefinitionNode>(std::move(name), std::move(fields),
                                                  std::move(methods), std::move(aliases), is_packed, is_templated);

    if (is_templated) {
        compiler->name_resolver.add_templated_class(cls, std::move(template_params), parent_type);
    } else {
        auto& info = class_info[cls->name];

        if (!info.name.empty())
            compiler->report_error("Redefinition of the class " + cls->name);

        info.is_templated = false;
        info.name = cls->name;
        info.parent = parent_type;
        info.node = cls;
    }

    current_class = "";
    in_templated_class = false;
}

void ParserAssistant::create_identifier_type()
{
    auto name = pop_str();
    auto args = pop_types();
    auto is_templated = pop_is_templated();
    if (is_templated)
        templated_class_definitions.insert({ name, args });
    push_type<IdentifierType>(std::move(name), is_templated, std::move(args));
}

void ParserAssistant::create_field_access() {
    auto template_args = pop_types();
    if (pop_is_templated())
        push_node<ClassFieldNode>(pop_node(), pop_str(), std::move(template_args));
    else
        push_node<ClassFieldNode>(pop_node(), pop_str());
}

void ParserAssistant::create_inferred_definition()
{
    push_type<TypeOfType>(nodes.back());
    create_variable_definition();
}

void ParserAssistant::create_size_of()
{
    push_node<SizeOfNode>(pop_type());
}

void ParserAssistant::create_type_of()
{
    push_type<TypeOfType>(pop_node());
}

void ParserAssistant::create_type_name()
{
    push_node<TypeNameNode>(pop_type());
}

void ParserAssistant::create_function_type()
{
    size_t param_count = pop_counter();
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

        compiler->name_resolver.register_function("malloc", {type, { "size" }}, true);

        declared_malloc = true;
    }

    auto args = pop_args();
    auto count = pop_node();
    push_node<MallocNode>(pop_type(), std::move(args), count, is_array, is_raw);
}

void ParserAssistant::create_free()
{
    if (!declared_free)
    {
        auto type = compiler->create_type<FunctionType> (
            compiler->create_type<VoidType>(),
            std::vector<const Type*>{ compiler->create_type<PointerType>(compiler->create_type<I64Type>()) }
        );

        compiler->name_resolver.register_function("free", { type, { "size" } }, true);

        declared_free = true;
    }
    push_node<FreeNode>(pop_node(), is_array, is_raw);
    inc_statements();
}

void ParserAssistant::create_pointer_field_access() {
    push_node<ClassFieldNode>(pop_node(), pop_str());
}

void ParserAssistant::prepare_constructor()
{
    if (!in_class())
        compiler->report_error("Constructors can only be defined inside classes");
    push_type<VoidType>();
    push_str("constructor");
    nomangle = false;
}

void ParserAssistant::finish_constructor()
{
    auto constructor = pop_node_as<FunctionDefinitionNode>();
    constructors_field_args.push_back({ constructor, std::move(fields_args), current_class, in_templated_class });
    fields_args.clear();
    nodes.push_back(constructor);
}

void ParserAssistant::prepare_destructor()
{
    if (!in_class())
        compiler->report_error("Destructors can only be defined inside classes");
    push_type<VoidType>();
    push_str("destructor");
    nomangle = false;
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
    bool is_templated = pop_is_templated();
    auto template_args = pop_types();
    auto param_types = pop_types();
    auto ret_type = pop_type();
    auto target_func = pop_resolution_string();
    auto is_var_arg = pop_var_arg();
    auto func_type = compiler->create_type<FunctionType>(ret_type, param_types, is_var_arg);
    push_type<PointerType>(func_type);
    if (is_templated)
        push_node<VariableNode>(target_func, std::move(template_args), types.back());
    else
        push_node<VariableNode>(target_func, types.back());

    push_counter();
    create_variable_definition();
}

void ParserAssistant::prepare_copy_constructor()
{
    if (!in_class())
        compiler->report_error("Copy constructors can only be defined inside classes");
    push_type<VoidType>();
    push_str("=constructor");
    nomangle = false;
}

void ParserAssistant::finish_copy_constructor()
{
    auto constructor = pop_node_as<FunctionDefinitionNode>();
    auto& param_types = constructor->get_function_type()->param_types;
    if (param_types.size() != 2)  // One for the instance and one for the other object
        compiler->report_error("Copy constructors must have exactly one parameter");
    constructors_field_args.push_back({ constructor, std::move(fields_args), current_class, in_templated_class});
    fields_args.clear();
    nodes.push_back(constructor);
}

void ParserAssistant::create_operator(const std::string& position_name)
{
    auto param_count = pop_counter();
    std::vector<const Type*> param_types(param_count);
    std::vector<std::string> param_names(param_count);

    inc_statements();

    for (int i = 0; i < param_count; i++) {
        param_names[param_count - i - 1] = pop_str();
        param_types[param_count - i - 1] = pop_type();
    }

    auto name = pop_str();

    bool is_method = in_class();
    if (param_count + is_method != 2) {
        if (is_method) {
            compiler->report_error("Class " + position_name + " operators expect exactly one parameter (in " + current_class + "::" + name + " operator)");
        } else {
            compiler->report_error("Global " + position_name + " operators expect exactly two parameters (in the global " + name + " operator)");
        }
    }

    auto return_type = pop_type();

    if (in_class()) {
        auto self_type = in_templated_class ? (const Type*)compiler->create_type<I64Type>() :
                    compiler->create_type<ReferenceType>(compiler->create_type<IdentifierType>(current_class), true);
        param_types.insert(
            param_types.begin(),
            self_type
        );
        param_names.insert(param_names.begin(), "self");
    }

    name = position_name + "." + name;

    auto function_type = compiler->create_type<FunctionType>(return_type, std::move(param_types), false);
    FunctionInfo info {
        function_type,
        std::move(param_names),
        false,
        in_templated_class ? nullptr : compiler->create_type<ClassType>(current_class)
    };

    push_node<FunctionDefinitionNode>(std::move(name), nullptr, function_type, false, -1, true, is_static);
    is_static = false;

    auto func = (FunctionDefinitionNode*)nodes.back();
    if (in_templated_class) {
        // Templated classes would add their own templated methods upon instantiation of concrete classes
        compiler->name_resolver.add_templated_class_method_info(current_class, func, std::move(info), {});
    } else {
        function_definitions.push_back({ func, std::move(info), current_class });
    }
}

void ParserAssistant::set_current_function()
{
    is_in_function = true;
}

void ParserAssistant::create_reference_type() {
    // Reference types start as allocated, and if the allocation is
    //  going to be optimized away, it'll be unallocated
    push_type<ReferenceType>(pop_type(), true);
}

void ParserAssistant::create_type_alias() {
    auto node = compiler->create_node<TypeAliasNode>(pop_str(), pop_type());
    if (is_in_global_scope()) {
        global_aliases.push_back(node);
    } else {
        nodes.push_back(node);
        inc_statements();
    }
}

void ParserAssistant::create_identifier_value()
{
    auto name = pop_resolution_string();
    auto template_args = pop_types();
    if (pop_is_templated()) {
        auto identity = dynamic_cast<IdentityResolutionString*>(name);
        assert(identity != nullptr);
        templated_class_definitions.insert({ identity->str, template_args });
        push_node<VariableNode>(name, std::move(template_args));
    } else {
        push_node<VariableNode>(name);
    }
}

void ParserAssistant::push_is_templated(bool is_templated) {
    is_templated_stack.push_back(is_templated);
}

bool ParserAssistant::pop_is_templated() {
    bool result = is_templated_stack.back();
    is_templated_stack.pop_back();
    return result;
}

void ParserAssistant::create_is_type() {
    push_node<IsTypeNode>(pop_type(), pop_type());
}

void ParserAssistant::create_no_ref() {
    push_type<NoRefType>(pop_type());
}

void ParserAssistant::create_infix_operator() {
    create_operator("infix");
}

void ParserAssistant::create_postfix_operator() {
    create_operator("postfix");
}

void ParserAssistant::create_method_identifier()
{
    auto class_type = pop_type();
    auto method = pop_str();
    // Identifier types may name types that are not yet defined. This
    //  prevents us from calling the to_string method on them because
    //  this may cause an undefined type error. Rather, we note that
    //  the name needs to be resolved further in the future.
    push_resolution_string<MethodNameResolutionString>(class_type, std::move(method));
}

void ParserAssistant::create_dynamic_name() {
    push_node<DynamicNameNode>(pop_node(), pop_type());
}

void ParserAssistant::create_dynamic_cast() {
    push_node<DynamicCastNode>(pop_node(), pop_type());
    used_dynamic_casting = true;
}

void ParserAssistant::create_address_of() {
    push_node<AddressOfNode>(pop_node());
}

void ParserAssistant::create_null_ptr() {
    push_node<NullPointerNode>();
}

void ParserAssistant::create_char_value() {
    // The result is "'c'" where c is the target char
    push_node<I8ValueNode>(escape_characters(pop_str())[1]);
}

void ParserAssistant::create_temp_object() {
    push_node<TempObjectNode>(pop_type(), pop_args());
}

void ParserAssistant::create_move() {
    push_node<MoveNode>(pop_str());
}

void ParserAssistant::create_offset_of() {
    push_node<OffsetOfNode>(pop_type(), pop_str());
}

}
