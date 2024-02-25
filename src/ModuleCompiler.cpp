#include <ModuleCompiler.hpp>
#include <AST/TranslationUnitNode.hpp>
#include "AST/function/FunctionDefinitionNode.hpp"
#include "AST/class/ClassDefinitionNode.hpp"
#include "AST/variable/ClassFieldDefinitionNode.hpp"
#include "types/PointerType.hpp"
#include "types/IntegerTypes.hpp"
#include "types/ReferenceType.hpp"
#include <parsing/ParserFacade.hpp>
#include "utils/CodeGeneration.hpp"
#include "AST/BlockNode.hpp"

#include <fstream>

#include <llvm/Support/Host.h>
#include <llvm/Transforms/Utils/ModuleUtils.h>
#include <llvm/IR/Verifier.h>

namespace dua
{

std::vector<std::string>& get_libdua_declarations();

ModuleCompiler::ModuleCompiler(std::string module_name, std::string code, bool include_libdua) :
    context(),
    module(module_name, context),
    builder(context),
    temp_builder(context),
    module_name(std::move(module_name)),
    name_resolver(this),
    typing_system(this),
    include_libdua(include_libdua),
    temp_expressions(this),
    code(std::move(code))
{
    module.setTargetTriple(llvm::sys::getDefaultTargetTriple());

    ParserFacade parser(*this);

    function_scope_count.push_back(0);

    create_the_object_class();

    create_dynamic_casting_function();

    create_dua_init_function();

    create_dua_cleanup_function();

    if (include_libdua) {
        for (auto &declarations: get_libdua_declarations())
            this->code += declarations;
    }

    // Parse
    TranslationUnitNode* ast = parser.parse(this->code);

    // Generate LLVM IR
    ast->eval();

    destruct_global_scope();

    // It's important that this function gets called
    //  before finalizing the .dua.init function,
    //  because the .dua.cleanup function is registered
    //  in the .dua.init function
    complete_dua_cleanup_function();

    complete_dua_init_function();

    llvm::raw_string_ostream stream(result);
    module.print(stream, nullptr);
    result = stream.str();

    std::ofstream outfile ("E:/test.txt");

    outfile << result << std::endl;

    outfile.close();
}

void ModuleCompiler::create_dua_init_function()
{
    dua_init_name = ".dua.init." + uuid();

    auto info = FunctionInfo {
        create_type<FunctionType>(create_type<VoidType>()),
        {},
        false,
        nullptr
    };

    name_resolver.register_function(dua_init_name, std::move(info), true);

    auto function = module.getFunction(dua_init_name);
    llvm::BasicBlock::Create(context, "entry", function);
}

void ModuleCompiler::complete_dua_init_function()
{
    auto dua_init = module.getFunction(dua_init_name);
    auto& init_ip = dua_init->getEntryBlock();
    builder.SetInsertPoint(&init_ip);
    for (auto node : deferred_nodes)
        node->eval();

    if (dua_init->begin()->begin() == dua_init->begin()->end()) {
        // The function is empty. Delete it for clarity.
        dua_init->removeFromParent();
    } else {
        // Return
        builder.CreateRetVoid();
        // TODO vary the priority as needed
        int priority = 0;
        llvm::appendToGlobalCtors(module, dua_init, priority);
    }
}

ModuleCompiler::~ModuleCompiler()
{
    for (auto node : nodes)
        delete node;
}

void ModuleCompiler::push_scope() {
    name_resolver.push_scope();
    typing_system.push_scope();
    function_scope_count.back()++;
}

Scope<Value> ModuleCompiler::pop_scope() {
    function_scope_count.back()--;
    typing_system.pop_scope();
    return name_resolver.pop_scope();
}

void ModuleCompiler::destruct_last_scope()
{
    if (builder.GetInsertBlock()->getTerminator() != nullptr)
        return;

    name_resolver.destruct_all_variables(name_resolver.symbol_table.scopes.back());
}

void ModuleCompiler::destruct_function_scope()
{
    if (builder.GetInsertBlock()->getTerminator() != nullptr)
        return;

    auto& scopes = name_resolver.symbol_table.scopes;
    auto n = scopes.size();
    for (size_t i = 1; i <= function_scope_count.back(); i++)
        name_resolver.destruct_all_variables(scopes[n - i]);
}

void ModuleCompiler::push_scope_counter() {
    function_scope_count.push_back(0);
}

void ModuleCompiler::pop_scope_counter() {
    function_scope_count.pop_back();
}

void ModuleCompiler::create_the_object_class()
{
    auto type = create_type<ClassType>("Object");

    std::vector<const Type*> params = { create_type<ReferenceType>(type, true) };
    auto info = FunctionInfo {
        create_type<FunctionType>(create_type<VoidType>(), params),
        { "self" },
        false,
        type
    };

    current_class = type->llvm_type();

    typing_system.insert_type("Object", type);
    name_resolver.classes["Object"] = type;

    BlockNode empty_block(this, {});

    name_resolver.register_function("Object.destructor", info);
    FunctionDefinitionNode destructor(this, "Object.destructor", &empty_block, info.type);
    destructor.eval();

    name_resolver.create_vtable("Object");
    name_resolver.class_fields["Object"].push_back(name_resolver.get_vtable_field("Object"));
    type->llvm_type()->setBody(name_resolver.get_vtable_type("Object")->llvm_type());

    name_resolver.register_function("Object.constructor", info);
    FunctionDefinitionNode constructor(this, "Object.constructor", &empty_block, info.type);
    constructor.eval();

    current_class = nullptr;
}

Value ModuleCompiler::create_string(const std::string &name, const std::string &value)
{
    auto type = create_type<PointerType>(create_type<I8Type>());
    auto it = string_pool.find(value);
    if (it != string_pool.end())
        return create_value(it->second, type);
    auto result = builder.CreateGlobalStringPtr(value, name, 0, &module);
    string_pool[value] = result;
    return create_value(result, type);
}

void ModuleCompiler::create_dynamic_casting_function()
{
    std::vector<const Type*> param_types = { create_type<PointerType>(create_type<ClassType>("Object")),
                         create_type<PointerType>(create_type<ClassType>("Object")) };
    auto info = FunctionInfo {
        create_type<FunctionType>(create_type<I8Type>(), param_types),
        {"init_vtable", "target_vtable"},
        false,
        nullptr
    };

    name_resolver.register_function(".is_vtable_reachable", std::move(info), true);

    auto function = module.getFunction(".is_vtable_reachable");

    // Loop while the current vtable instance pointer is not the target vtable pointer, or if
    //  you reach the root (a null pointer)
    //  This is equivalent to:
    //      auto ptr = vtable_ptr;
    //      while (ptr && ptr != target_ptr)
    //          ptr = parent_vtable(ptr);
    //      if (ptr == null) return null
    //      return pointer as target_type

    auto entry_bb = llvm::BasicBlock::Create(context, "entry", function);
    auto test_null_bb = llvm::BasicBlock::Create(context, "test_null", function);
    auto proceed_bb = llvm::BasicBlock::Create(context, "proceed", function);
    auto load_parent_bb = llvm::BasicBlock::Create(context, "load_parent", function);
    auto end_bb = llvm::BasicBlock::Create(context, "end", function);

    auto any_vtable = name_resolver.get_vtable_instance("Object");

    auto init_vtable = function->args().begin();
    auto target_vtable = function->args().begin() + 1;

    // Even though vtables of different classes are different in structure, the only part
    //  we're concerned with is the pointer of the parent vtable instance, which is always
    //  in the same position, regardless of the vtable type. For this reason, if we're only
    //  accessing the parent vtable pointer, we can treat any vtable instance as any vtable.
    auto vtable_llvm_type = init_vtable->getType();

    builder.SetInsertPoint(entry_bb);

    auto current_vtable_ptr = builder.CreateAlloca(vtable_llvm_type, nullptr, "current_vtable");
    builder.CreateStore(init_vtable, current_vtable_ptr);

    builder.CreateBr(test_null_bb);

    builder.SetInsertPoint(test_null_bb);

    llvm::Value* current_vtable = builder.CreateLoad(vtable_llvm_type->getPointerTo(), current_vtable_ptr);
    current_vtable = builder.CreatePointerCast(current_vtable, target_vtable->getType());

    auto is_null = builder.CreateICmpNE(current_vtable, llvm::Constant::getNullValue(current_vtable->getType()));
    builder.CreateCondBr(is_null, proceed_bb, end_bb);

    builder.SetInsertPoint(proceed_bb);

    auto is_target = builder.CreateICmpEQ(current_vtable, target_vtable);
    builder.CreateCondBr(is_target, end_bb, load_parent_bb);

    builder.SetInsertPoint(load_parent_bb);

    // Any vtable is usable here
    auto parent_ptr = any_vtable->get_ith_element(1, vtable_llvm_type, current_vtable);
    builder.CreateStore(parent_ptr, current_vtable_ptr);
    builder.CreateBr(test_null_bb);

    builder.SetInsertPoint(end_bb);

    auto phi = builder.CreatePHI(builder.getInt1Ty(), 2, "result");
    phi->addIncoming(builder.getInt1(1), proceed_bb);
    phi->addIncoming(builder.getInt1(0), test_null_bb);

    auto result = builder.CreateZExt(phi, builder.getInt8Ty());

    builder.CreateRet(result);

    auto comdat = module.getOrInsertComdat(".is_vtable_reachable");
    comdat->setSelectionKind(llvm::Comdat::Any);
    function->setComdat(comdat);
}

void ModuleCompiler::delete_dynamic_casting_function() {
    auto function = module.getFunction(".is_vtable_reachable");
    function->eraseFromParent();
}

// Wrapper functions for easier name resolution
static void _report_error(const std::string& message) { report_error(message); }
static void _report_internal_error(const std::string& message) { report_internal_error(message); }
static void _report_warning(const std::string& message) { report_warning(message); }

void ModuleCompiler::report_error(const std::string &message) {
    _report_error(get_current_status() + message);
}

void ModuleCompiler::report_internal_error(const std::string &message) {
    _report_internal_error(get_current_status() + message);
}

void ModuleCompiler::report_warning(const std::string &message) {
    _report_warning(get_current_status() + message);
}

void report_error(const std::string& message, ModuleCompiler* compiler)
{
    compiler->report_error(message);
}

void report_internal_error(const std::string& message, ModuleCompiler* compiler)
{
    compiler->report_internal_error(message);
}

void report_warning(const std::string& message, ModuleCompiler* compiler)
{
    compiler->report_warning(message);
}

std::string ModuleCompiler::get_current_status()
{
    // FIXME this information is misleading sometimes since
    //  it reports the current scope the compiler is at, and
    //  not where did the compiler encounter the error in the
    //  code. This may give false information for example if
    //  the compiler is in the global scope, scanning for
    //  function headers, and encounter a non-defined type in
    //  the signature. In this case, the message will report
    //  that the error is in the global scope, where in fact,
    //  the error is in one of the functions.

    std::string result = "In file " + module_name;
    result.pop_back();
    result.pop_back();
    result += "dua";

    if (current_class != nullptr) {
        auto class_name = current_class->getName().str();
        result += ", in class " + class_name;
    }

    if (current_function != nullptr) {
        auto func_name = current_function->getName().str();
        result += ", in function " + func_name;
    }

    if (current_class == nullptr && current_function == nullptr) {
        result += ", in global scope";
    }

    result += ":\n";

    return result;
}

llvm::Function *ModuleCompiler::get_dua_init_function() {
    return module.getFunction(dua_init_name);
}

void ModuleCompiler::create_dua_cleanup_function()
{
    dua_cleanup_name = ".dua.cleanup." + uuid();

    auto info = FunctionInfo {
        create_type<FunctionType>(create_type<VoidType>()),
        {},
        false,
        nullptr
    };

    name_resolver.register_function(dua_cleanup_name, std::move(info), true);

    auto function = module.getFunction(dua_cleanup_name);
    llvm::BasicBlock::Create(context, "entry", function);
}

void ModuleCompiler::complete_dua_cleanup_function()
{
    auto dua_cleanup = module.getFunction(dua_cleanup_name);
    auto& cleanup_ip = dua_cleanup->getEntryBlock();
    builder.SetInsertPoint(&cleanup_ip);
    if (dua_cleanup->begin()->begin() == dua_cleanup->begin()->end()) {
        // The function is empty. Delete it for clarity.
        dua_cleanup->removeFromParent();
    } else {
        // Return
        builder.CreateRetVoid();

        // Using @llvm.global_dtors crashes on some Windows machines. As a workaround,
        //  we register the cleanup functions using the atexit function instead.

        // Declare the atexit function
        auto atexit_type = create_type<FunctionType> (
            create_type<VoidType>(),
            std::vector<const Type*> { create_type<PointerType>(create_type<I64Type>()) }
        );
        llvm::Function* atexit_func = llvm::Function::Create(atexit_type->llvm_type(), llvm::Function::ExternalLinkage, "atexit", module);
        llvm::verifyFunction(*atexit_func);

        auto init = get_dua_init_function();
        builder.SetInsertPoint(&init->getEntryBlock());
        builder.CreateCall(atexit_func, dua_cleanup);
    }
}

llvm::Function *ModuleCompiler::get_dua_cleanup_function() {
    return module.getFunction(dua_cleanup_name);
}

void ModuleCompiler::destruct_global_scope()
{
    // Destruct global variables in the .dua.cleanup function
    builder.SetInsertPoint(&get_dua_cleanup_function()->getEntryBlock());

    auto global_scope = name_resolver.symbol_table.scopes[0].map;
    for (auto& [name, value] : global_scope)
    {
        auto type = value.type->get_concrete_type();
        // Type::as discards references.
        auto as_class = dynamic_cast<const ClassType*>(type);
        if (as_class == nullptr) continue;

        // Call the destructor directly, without loading it from the vtable
        auto destructor_name = name_resolver.get_function_full_name(as_class->name + ".destructor", false);
        auto destructor = module.getFunction(destructor_name);
        builder.CreateCall(destructor, { value.get() });
    }
}

int64_t ModuleCompiler::get_temp_expr_map_unused_id()
{
    return next_temp_expr_id++;
}

void ModuleCompiler::insert_temp_expr(const Value& value)
{
    if (temp_expressions.scopes.back().contains(value.id))
        report_internal_error("Collision in the temp expression map with id = " + std::to_string(value.id));
    temp_expressions.insert(value.id, value);
}

void ModuleCompiler::remove_temp_expr(int64_t id, bool panic_if_not_found)
{
    if (!temp_expressions.scopes.back().contains(id)) {
        if (panic_if_not_found)
            report_internal_error("There is no value with the id " + std::to_string(id) + " in the temp expression map");
    } else {
        temp_expressions.scopes.back().erase(id);
    }
}

void ModuleCompiler::push_temp_expr_scope() {
    temp_expressions.push_scope();
}

void ModuleCompiler::destruct_temp_expr_scope()
{
    for (auto& [id, value] : temp_expressions.scopes.back().map) {
        // Creating a value with the pointer. This is what call_destructor accepts
        auto ptr = create_value(value.memory_location, value.type);
        name_resolver.call_destructor(ptr);
    }
    temp_expressions.pop_scope();
}

Value ModuleCompiler::get_bound_value(Value value, const Type *bound_type)
{
    // If bound_type = nullptr, then this value is passed as a
    //  variadic argument. Substitute the type of the value instead

    // Only remove it if it bounds to a non-reference type
    // This handles the case where bound_type = nullptr as well.
    if (dynamic_cast<const ReferenceType*>(bound_type) == nullptr) {
        remove_temp_expr(value.id);
    }

    // Strip the reference if exists since
    // variadic args can't be of a reference type
    if (bound_type == nullptr)
        bound_type = value.type->get_contained_type();

    if (!value.is_teleporting)
    {
        auto class_type = bound_type->get_concrete_type()->is<ClassType>();
        if (class_type != nullptr)
        {
            // This variables won't be inserted in the symbol table, because they
            //  are supposed to live in the scope of the function getting called.
            // The function will put them in the symbol table, and will destroy them
            //  as needed at its scope.
            llvm::BasicBlock* entry = &current_function->getEntryBlock();
            temp_builder.SetInsertPoint(entry, entry->begin());
            auto ptr = temp_builder.CreateAlloca(class_type->llvm_type());
            auto ptr_value = create_value(ptr, class_type);
            name_resolver.copy_construct(ptr_value, value);
            value.memory_location = ptr;
            value.set(nullptr);
        }
    }

    return value;
}

llvm::AllocaInst* ModuleCompiler::create_local_variable(const std::string& name, const Type* type, Value* init, std::vector<Value> args, bool track_variable)
{
    llvm::BasicBlock* entry = &current_function->getEntryBlock();
    temp_builder.SetInsertPoint(entry, entry->begin());
    llvm::AllocaInst* instance = temp_builder.CreateAlloca(type->llvm_type(), 0, name);
    auto value = create_value(instance, type->get_concrete_type());

    if (!type->as<ReferenceType>()) {
        // Initializing the variable to a clean zero value
        //  before the constructor is called (if it's a class type)
        builder.CreateStore(type->zero_value().get(), instance);
    }

    if (track_variable && !name.empty())
        name_resolver.symbol_table.insert(name, value);

    if (init)
    {
        if (!args.empty()) {
            report_error("Can't have an both an initializer and an initialization "
                                 "list in the definition of a local variable (the variable " + name + ")");
        }
        name_resolver.copy_construct(value, *init);
    } else {
        name_resolver.call_constructor(value, std::move(args));
    }

    return instance;
}

llvm::BasicBlock* ModuleCompiler::create_basic_block(const std::string& name, llvm::Function* function) {
    if (function == nullptr) function = current_function;
    return llvm::BasicBlock::Create(context, name, function);
}

std::vector<std::string> libdua_declarations {

// Templated classes have to have the whole definitions included,
//  because the declaration alone doesn't instantiate a concrete class.
R"(

class Vector<T>
{
    typealias size_t = long;

    size_t _size = 0;
    size_t _capacity;
    T* buffer;
    bool _is_const_initialized = false;

    constructor(size_t n) : _capacity(n), buffer(n > 0 ? new[n] T : null)
    {
        if (n < 0) panic("The Vector class can't have negative size");
    }

    constructor(T* buffer, size_t size) : _size(size)
    {
        constructor(size);
        for (size_t i = 0; i < size; i++)
            self.buffer[i] = buffer[i];
    }

    constructor(T* buffer, size_t size, bool is_const, bool is_owned) : _is_const_initialized(is_const), _size(size)
    {
        if (is_const || is_owned) {
            self.buffer = buffer;
            _capacity = size;
        } else {
            constructor(buffer, size);
        }
    }

    constructor(size_t n, T value)
    {
        constructor(n);
        for (size_t i = 0; i < n; i++)
            buffer[i] = value;
        _size = n;
    }

    constructor() { constructor(2); }

    =constructor(Vector<T>& other)
    {
        self = other;
    }

    size_t size() { return _size; }

    size_t capacity() { return _capacity; }

    void push(T t)
    {
        expand_if_needed();
        buffer[_size++] = t;
    }

    T pop()
    {
        if (_size == 0)
            panic("Can't pop an empty vector");

        // If the buffer refers to a constant memory,
        //  a new buffer must be allocated to modify it
        if (_is_const_initialized)
            alloc_new_buffer(_capacity);

        // TODO call the destructor on the popped element
        return buffer[--_size];
    }

    T& postfix [](size_t i)
    {
        if (i < 0)
            panic("Can't have a negative index\n");

        if (i >= size())
            panic("Can't have an index bigger than the size\n");

        //  but as of now, there is no way to tell whether the returned
        //  reference is going to be read from or written to, so this
        //  step is needed. Nevertheless, this would be useful for example
        //  in case of temporary string literals, which are just a temporary
        //  objects that won't be modified. Instead of copying them, wrap
        //  them with a String object without copying.
        if (_is_const_initialized)
            alloc_new_buffer(_capacity);

        return buffer[i];
    }

    void expand_if_needed()
    {
        if (_size == _capacity || _is_const_initialized)
            alloc_new_buffer(_size * 2);
    }

    void trim_to_fit()
    {
        if (_size > 0)
            alloc_new_buffer(_size);
    }

    void alloc_new_buffer(size_t new_capacity)
    {
        if (new_capacity <= 0)
            panic("Cannot allocate a non-positive-sized buffer");

        T* temp = new[new_capacity] T;

        for (size_t i = 0; i < _size; i++)
            temp[i] = buffer[i];

        _destroy_buffer();
        buffer = temp;

        _capacity = new_capacity;

        _is_const_initialized = false;
    }

    void resize(size_t new_size)
    {
        if (new_size <= 0)
            panic("Cannot resize to a non-positive size");

        if (new_size > _capacity)
            alloc_new_buffer(new_size);

        _size = new_size;
    }

    void reserve(size_t amount)
    {
        if (amount <= 0)
            panic("Cannot reserve a non-positive amount");

        if (amount <= _capacity)
            return;

        alloc_new_buffer(amount);
    }

    Vector<T>& infix =(Vector<T>& other)
    {
        if (&other == &self) return self;

        if (other._is_const_initialized)
        {
            _destroy_buffer();
            buffer = other.buffer;
            _size = other._size;
            _capacity = other._capacity;
        }
        else if (other._size != 0)
        {
            resize(other._size);

            // Don't set _size and _capacity. the resize method will set
            //  them as appropriate (_capacity might remain bigger than
            //  other._capacity)

            for (size_t i = 0; i < _size; i++)
                buffer[i] = other.buffer[i];
        }

        _is_const_initialized = other._is_const_initialized;

        return self;
    }

    // Sorting can't be embed in the vector class because this
    //  would require types stored in the class to have the
    //  operators necessary for comparison be defined, which
    //  is not needed if the class is going to be used just
    //  for storing elements, regardless of their order.
    void sort(int(T*, T*)* comparator) {
        sort<T>(buffer, size(), comparator);
    }

    void shuffle() {
        shuffle<T>(buffer, size());
    }

    void reverse() {
        reverse<T>(buffer, size());
    }

    bool is_empty() { return size() == 0; }

    destructor
    {
        _destroy_buffer();
    }

    void destruct_elements()
    {
        // Const pointers are in a read-only
        //  memory, and shouldn't be deleted
        if (!_is_const_initialized)
        {
            if ((&buffer[0] as Object*) != null) {
                // Have to cast as Object* so that the typing
                //  system doesn't complain if T is a primitive type
                for (size_t i = 0; i < size(); i++) {
                    var obj = ((Object*))&buffer[i];
                    obj->destructor();
                }
            }
        }
    }

    void clear()
    {
        destruct_elements();
        _size -= size();
    }

    void remove(size_t index)
    {
        if (index < 0)
            panic("Can't remove a negative index");

        if (index >= size())
            panic("Can't remove an index bigger than the size");

        for (size_t i = index; i < _size - 1; i++) {
            buffer[i] = buffer[i + 1];
        }

        _size--;
    }

    void _destroy_buffer()
    {
        // Const pointers are in a read-only
        //  memory, and shouldn't be deleted
        if (_is_const_initialized)
            return;

        destruct_elements();

        // Deleting with _RAW_ instead of delete to avoid
        //  calling the destructor on every position
        //  in the buffer, even if it's not occupied
        _RAW_ delete[] buffer;
    }
}

void panic(str message);

)"
,
R"(

class String : Vector<i8>
{
    typealias size_t = long;

    constructor();
    constructor(str string);
    constructor(size_t n, byte c);
    constructor(str string, size_t n, bool is_const, bool is_owned);
    constructor(str string, bool is_const, bool is_owned);

    =constructor(String& string);
    =constructor(str string);

    Declaration void push(byte c);
    Declaration byte pop();
    Declaration String substring(size_t from, size_t upto);

    Declaration size_t size();
    Declaration size_t capacity();

    Declaration str c_str();

    String& infix =(str string);
    String infix +(String& other);
    String infix +(str other);
    String infix +=(str other);
    String infix +=(String& other);

    destructor;
}

class InputStream;

class OutputStream;

InputStream& infix >>(InputStream& stream, String& string);

OutputStream& infix <<(OutputStream& stream, String& string);

String infix +(str other, String& self);

)"
,
R"(

class InputStream
{
    int* stream;
    str buffer;
    bool end_of_file_error = false;
    bool input_failure_error = false;
    bool range_error = false;  // Underflow or Overflow
    bool invalid_input_error = false;

    constructor(int* stream);

    constructor();

    Declaration void reset_flags();

    InputStream& infix >>(char& c);

    InputStream& infix >>(i16& num);

    InputStream& infix >>(i32& num);

    InputStream& infix >>(i64& num);

    InputStream& infix >>(f32& num);

    InputStream& infix >>(f64& num);

    Declaration int scan_buffer(byte* buffer, long size);
}

class OutputStream
{
    int* stream;
    bool error = false;
    int error_code = 0;

    constructor(int* stream);

    constructor();

    OutputStream& infix <<(char c);

    OutputStream& infix <<(i64 num);

    OutputStream& infix <<(double num);

    OutputStream& infix <<(str string);
}

extern InputStream in;

extern OutputStream out;

extern OutputStream err;

)"
,
R"(

Declaration void set_random_seed(i32 seed);

Declaration void set_random_seed();

Declaration i16 random_int();

)"
,
R"(

void sort<T>(T* base, long n, int(T*, T*)* comparator)
{
    qsort(((int*))base, n, sizeof(T), ((int(int*, int*)*))comparator);
}

int ascending_comparator<T>(T* a, T* b)
{
    return *a - *b;
}

int descending_comparator<T>(T* a, T* b)
{
    return *b - *a;
}

void sort_ascending<T>(T* base, long n)
{
    sort<T>(base, n, ascending_comparator<T>);
}

void sort_descending<T>(T* base, long n)
{
    sort<T>(base, n, descending_comparator<T>);
}

int random_comparator<T>(T* a, T* b)
 {
    return (random_int() % 2) ? 1 : -1;
}

void shuffle<T>(T* base, long n)
{
    sort<T>(base, n, random_comparator<T>);
}

void reverse<T>(T* base, long n)
{
    long i = 0;
    long j = n - 1;
    while (i < j)
    {
        T temp = base[i];
        base[i] = base[j];
        base[j] = temp;
        i++;
        j--;
    }
}

nomangle void qsort(int* base, long n, long size, int(int*, int*)* comparator);

)"
,
R"(

class PriorityQueue<T>
{
    typealias size_t = long;

    Vector<T> array;
    int(T*, T*)* comparator;

    constructor(int(T*, T*)* comparator) : comparator(comparator) { }

    size_t parent(size_t child) = (child - 1) / 2;

    size_t left(size_t parent) = parent * 2 + 1;

    size_t right(size_t parent) = left(parent) + 1;

    void swap(size_t i, size_t j)
    {
        T temp = array[i];
        array[i] = array[j];
        array[j] = temp;
    }

    // This method won't get called if the array is
    // empty because it gets called after an insertion
    size_t back_index() = array.size() - 1;

    void bubble_up(size_t i)
    {
        if (i == 0) return;
        size_t p = parent(i);
        if (comparator(&array[p], &array[i]) > 0) {
            swap(i, p);
            bubble_up(p);
        }
    }

    void bubble_down(size_t i)
    {
        size_t l = left(i);
        size_t r = right(i);
        if (l >= array.size()) {
            return;
        } else if (r >= array.size()) {
            // here, l is within the bounds
            if (comparator(&array[l], &array[i]) < 0) {
                swap(i, l);
                bubble_down(l);
            }
        } else {
            // Both l and r are within the bounds
            int cl = comparator(&array[l], &array[i]);
            int cr = comparator(&array[r], &array[i]);
            if (cl < 0 || cr < 0) {
                if (comparator(&array[l], &array[r]) < 0) {
                    swap(i, l);
                    bubble_down(l);
                } else {
                    swap(i, r);
                    bubble_down(r);
                }
            }
        }
    }

    void insert(T t)
    {
        array.push(move(t));
        bubble_up(back_index());
    }

    T& peek()
    {
        if (array.is_empty())
            panic("Can't peek in an empty heap");
        return array[0];
    }

    T pop()
    {
        if (array.is_empty())
            panic("Can't pop from an empty heap");

        if (array.size() == 1)
            return array.pop();

        T result = array[0];
        array[0] = array.pop();
        bubble_down(0);
        return result;
    }

    size_t size() { return array.size(); }

    bool is_empty() { return array.is_empty(); }
}

class MinPriorityQueue<T> : PriorityQueue<T>
{
    constructor() : Super(ascending_comparator<T>) { }
}

class MaxPriorityQueue<T> : PriorityQueue<T>
{
    constructor() : Super(descending_comparator<T>) { }
}

)"
,
R"(

nomangle void exit(int exit_code);

nomangle void getenv(str name);

nomangle int system(str command);

void panic(str message);

)"
,
R"(

nomangle int* c_stdin ();
nomangle int* c_stdout();
nomangle int* c_stderr();

nomangle int c_errno();

nomangle int c_EOF();
nomangle int c_ERANGE();

nomangle int c_scan_str(int* stream, str buffer, int* read_count);

)"

};

std::vector<std::string>& get_libdua_declarations() { return libdua_declarations; }

}
