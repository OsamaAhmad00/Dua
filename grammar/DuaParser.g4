parser grammar DuaParser;

options {
	tokenVocab = DuaLexer;
}

@parser::postinclude {
#include <string>
#include <AST/all.h>
#include <ModuleCompiler.h>
}

@parser::members
{
private:

    ModuleCompiler* module_compiler;

public:

    void set_module_compiler(ModuleCompiler* module_compiler) {
        this->module_compiler = module_compiler;
    }

    template <typename T, typename ...Args>
    T* create(Args... args) {
        return module_compiler->create_node<T>(args...);
    }
}

entry_point
returns [TranslationUnitNode* result]
@init{ $result = create<TranslationUnitNode>(); }
    :  EOF
    ;
