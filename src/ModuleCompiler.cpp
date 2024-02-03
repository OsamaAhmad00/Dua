#include <ModuleCompiler.hpp>
#include <parsing/ParserFacade.hpp>
#include <AST/TranslationUnitNode.hpp>
#include <llvm/Support/Host.h>
#include "AST/function/FunctionDefinitionNode.hpp"
#include "AST/class/ClassDefinitionNode.hpp"
#include "types/ReferenceType.hpp"
#include "AST/variable/ClassFieldDefinitionNode.hpp"
#include "types/PointerType.hpp"
#include "types/IntegerTypes.hpp"

namespace dua
{

std::vector<std::string>& get_dua_lib_declarations();

ModuleCompiler::ModuleCompiler(const std::string &module_name, std::string code, bool append_declarations) :
    context(),
    module(module_name, context),
    builder(context),
    temp_builder(context),
    name_resolver(this),
    typing_system(this)
{
    module.setTargetTriple(llvm::sys::getDefaultTargetTriple());

    ParserFacade parser(*this);

    function_scope_count.push_back(0);

    create_the_object_class();

    create_dynamic_casting_function();

    create_dua_init_function();

    if (append_declarations) {
        for (auto &declarations: get_dua_lib_declarations())
            code += declarations;
    }

    // Parse
    TranslationUnitNode* ast = parser.parse(code);

    // Generate LLVM IR
    ast->eval();

    complete_dua_init_function();

    llvm::raw_string_ostream stream(result);
    module.print(stream, nullptr);
    result = stream.str();
}

void ModuleCompiler::create_dua_init_function()
{
    auto info = FunctionInfo {
        create_type<FunctionType>(create_type<VoidType>()),
        {}
    };

    name_resolver.register_function(".dua.init", std::move(info), true);

    auto function = module.getFunction(".dua.init");
    llvm::BasicBlock::Create(context, "entry", function);

    // TODO delete this
    auto comdat = module.getOrInsertComdat(".dua.init");
    comdat->setSelectionKind(llvm::Comdat::Any);
    function->setComdat(comdat);
}

void ModuleCompiler::complete_dua_init_function()
{
    auto dua_init = module.getFunction(".dua.init");
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

        // Call from main.
        // TODO remove this and create an appending
        //  global array variable with all initializing
        //  methods to be called
        auto main = module.getFunction("main");
        if (main != nullptr) {
            auto &main_ip = main->getEntryBlock().front();
            builder.SetInsertPoint(&main_ip);
            builder.CreateCall(dua_init);
        }
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
    std::vector<const Type*> params = { create_type<ReferenceType>(create_type<ClassType>("Object"), true) };
    auto info = FunctionInfo {
            create_type<FunctionType>(create_type<VoidType>(), params),
            {}
    };

    // The destructor name should be mangled so that it's compliant with the rest
    //  of the methods, and overriding happens successfully
    name_resolver.register_function("Object.destructor", std::move(info));
    auto destructor = module.getFunction("Object.destructor(Object&)");
    auto bb = llvm::BasicBlock::Create(context, "entry", destructor);
    auto ip = builder.saveIP();
    builder.SetInsertPoint(bb);
    builder.CreateRetVoid();
    builder.restoreIP(ip);

    auto comdat = module.getOrInsertComdat("Object.destructor(Object&)");
    comdat->setSelectionKind(llvm::Comdat::Any);
    destructor->setComdat(comdat);

    auto type = create_type<ClassType>("Object");
    typing_system.insert_type("Object", type);
    name_resolver.classes["Object"] = type;
    name_resolver.create_vtable("Object");
    name_resolver.class_fields["Object"].push_back(name_resolver.get_vtable_field("Object"));
    type->llvm_type()->setBody(name_resolver.get_vtable_type("Object")->llvm_type());
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
        {"init_vtable", "target_vtable"}
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

    // TODO delete this
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

    std::string result;

    if (current_class != nullptr) {
        auto class_name = current_class->getName().str();
        result += "in class " + class_name;
    }

    if (current_function != nullptr) {
        auto func_name = current_function->getName().str();
        if (!result.empty())
            result += ", ";
        result += "in function " + func_name;
    }

    if (result.empty()) {
        result = "In global scope";
    }

    result[0] = toupper(result[0]);

    result += ":\n";

    return result;
}

std::vector<std::string> dua_lib_declarations {

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
        if (_size < 0) panic("The Vector class can't have negative size");
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

        if (i >= _size)
            panic("Can't have an index bigger than the size\n");

        // FIXME this defeats the purpose of the use of the const buffer,
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
        if (other._is_const_initialized)
        {
            _destroy_buffer();
            buffer = other.buffer;
            _size = other._size;
            _capacity = other._capacity;
        }
        else
        {
            resize(other._size);

            // Don't set _capacity. the resize method will set it as
            // appropriate (it might remain bigger than other._capacity)
            _size = other._size;

            for (size_t i = 0; i < _size; i++)
                buffer[i] = other.buffer[i];
        }

        _is_const_initialized = other._is_const_initialized;

        return self;
    }

    bool is_empty() { return size() == 0; }

    destructor
    {
        _destroy_buffer();
    }

    void _destroy_buffer()
    {
        // Const pointers are in a read-only
        //  memory, and shouldn't be deleted
        if (_is_const_initialized)
            return;

        if ((&buffer[0] as Object*) != null) {
            // Have to cast as Object* so that the typing
            //  system doesn't complain if T is a primitive type
            for (size_t i = 0; i < size(); i++) {
                var obj = ((Object*))&buffer[i];
                obj->destructor();
            }
        }

        // Calling free instead of delete to avoid calling
        //  the destructor again on every position in the
        //  buffer, even if it's not occupied
        free(((i64*))buffer);
    }
}

void panic(str message);
nomangle void free(i64* ptr);

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
    String infix +(String other);
    String infix +(str other);

    destructor;
}

String infix +(str other, String self);

)"
,
R"(

// TODO Remove the "c_" prefix after solving the problem
//  of the .dua.init function, and make them static
int* c_stdin  = __get_stdin ();
int* c_stdout = __get_stdout();
int* c_stderr = __get_stderr();

nomangle int* __get_stdin();

nomangle int* __get_stdout();

nomangle int* __get_stderr();

nomangle void fprintf(int* stream, str message, ...);

nomangle void exit(int exit_code);

nomangle void getenv(str name);

nomangle int system(str command);

void panic(str message);

)"
};

std::vector<std::string>& get_dua_lib_declarations() { return dua_lib_declarations; }

}
