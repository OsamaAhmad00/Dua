#pragma once

#include <unordered_map>
#include <string>
#include <vector>
#include <stdexcept>
#include <utils/ErrorReporting.hpp>

namespace dua
{

template<typename T>
struct Scope
{
    Scope(ModuleCompiler* compiler) : compiler(compiler) {}

    Scope(const Scope& scope) : compiler(scope.compiler), map(scope.map) {}

    Scope(Scope&& scope) : compiler(scope.compiler), map(std::move(scope.map)) {}

    Scope& operator=(const Scope& scope) {
        if (&scope != this) {
            compiler = scope.compiler;
            map = scope.map;
        }
        return *this;
    }

    Scope& operator=(Scope&& scope) {
        if (&scope != this) {
            compiler = scope.compiler;
            map = std::move(scope.map);
        }
        return *this;
    }

    ModuleCompiler* compiler;
    std::unordered_map<std::string, T> map;

    Scope& insert(const std::string& name, const T& t) {
        if (contains(name)) {
            report_error("The symbol " + name + " is already defined", compiler);
        }
        map[name] = t;
        return *this;
    }

    const T& get(const std::string& name) {
        if (!contains(name)) {
            report_error("The symbol " + name + " is not defined", compiler);
        }
        return map[name];
    }

    bool contains(const std::string& name) const {
        return map.find(name) != map.end();
    }
};

template<typename T>
struct SymbolTable
{
    ModuleCompiler* compiler;
    std::vector<Scope<T>> scopes;
    // Used when switching to a temporary state in which
    //  all scopes but the global one are discarded.
    std::vector<std::vector<Scope<T>>> switch_stack;

    SymbolTable(ModuleCompiler* compiler) : compiler(compiler) { push_scope(); }

    size_t size() const { return scopes.size(); }

    SymbolTable& insert(const std::string& name, const T& t, int scope_num = 1) {
        if (scopes.empty()) {
            report_internal_error("There is no scope", compiler);
        }
        auto& scope = scopes[scopes.size() - scope_num];
        scope.insert(name, t);
        return *this;
    }

    SymbolTable& insert_global(const std::string& name, const T& t) {
        insert(name, t, size());
        return *this;
    }

    const T& get(const std::string& name, bool include_global = true, int scope_num = 1) {
        if (scopes.empty()) {
            report_internal_error("There is no scope", compiler);
        }

        for (int i = scopes.size() - scope_num; i >= 1 + !include_global; i--) {
            if (scopes[i].contains(name)) {
                return scopes[i].get(name);
            }
        }

        // no check here so that it errs if not present here either.
        return scopes[!include_global].get(name);
    }

    const T& get_global(const std::string& name) {
        return get(name, true, size());
    }

    bool contains(const std::string& name, bool include_global = true, int scope_num = 1) {
        if (scopes.empty()) {
            report_internal_error("There is no scope", compiler);
        }

        for (int i = scopes.size() - scope_num; i >= !include_global; i--) {
            if (scopes[i].contains(name)) {
                return true;
            }
        }

        return false;
    }

    bool contains_global(const std::string& name) {
        return contains(name, true, size());
    }

    SymbolTable& push_scope(Scope<T> scope) {
        scopes.push_back(std::move(scope));
        return *this;
    }

    SymbolTable& push_scope() {
        push_scope(Scope<T>(compiler));
        return *this;
    }

    Scope<T> pop_scope() {
        Scope<T> top = std::move(scopes.back());
        scopes.pop_back();
        return top;
    }

    void keep_only_last_n_scopes(size_t n, bool include_global_scope = true)
    {
        if ((n + include_global_scope) > scopes.size())
            report_internal_error("Can't keep " + std::to_string(n + include_global_scope) + " scopes. There are only " + std::to_string(scopes.size()) + " scopes", compiler);

        switch_stack.push_back(std::move(scopes));

        scopes.resize(n + include_global_scope, Scope<T>(compiler));

        if (include_global_scope)
            scopes[0] = switch_stack.back()[0];

        auto offset = switch_stack.back().size() - 1 - n;
        for (size_t i = include_global_scope; i < n; i++)
            scopes[i] = switch_stack.back()[offset + i];
    }

    void restore_prev_state()
    {
        // Global scope is kept
        auto global_scope = scopes.front();
        scopes = std::move(switch_stack.back());
        scopes.front() = std::move(global_scope);
        switch_stack.pop_back();
    }

    void remove_last_occurrence_of(const std::string& name, bool panic_if_not_found = true)
    {
        for (size_t i = size() - 1; i != size_t(-1); i--) {
            if (scopes[i].contains(name)) {
                scopes[i].map.erase(name);
                return;
            }
        }

        if (panic_if_not_found)
            report_internal_error("The symbol " + name + " is not present in the symbol table");
    }
};

}
