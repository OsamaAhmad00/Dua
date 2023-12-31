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
    std::unordered_map<std::string, T> map;

    Scope& insert(const std::string& name, const T& t) {
        if (contains(name)) {
            report_error("The symbol " + name + " is already defined");
        }
        map[name] = t;
        return *this;
    }

    const T& get(const std::string& name) {
        if (!contains(name)) {
            report_error("The symbol " + name + " is not defined");
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
    std::vector<Scope<T>> scopes;
    // Used when switching to a temporary state in which
    //  all scopes but the global one are discarded.
    std::vector<std::vector<Scope<T>>> switch_stack;

    SymbolTable() { push_scope(); }

    size_t size() const { return scopes.size(); }

    SymbolTable& insert(const std::string& name, const T& t, int scope_num = 1) {
        if (scopes.empty()) {
            report_internal_error("There is no scope");
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
            report_internal_error("There is no scope");
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
            report_internal_error("There is no scope");
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
        push_scope(Scope<T>());
        return *this;
    }

    Scope<T> pop_scope() {
        Scope<T> top = std::move(scopes.back());
        scopes.pop_back();
        return top;
    }

    void keep_only_first_n_scopes(size_t n)
    {
        if (n > scopes.size())
            report_internal_error("Can't keep " + std::to_string(n) + " scopes. There are only " + std::to_string(scopes.size()) + " scopes");

        switch_stack.push_back(std::move(scopes));

        scopes.resize(n);
        for (size_t i = 0; i < n; i++)
            scopes[i] = switch_stack.back()[i];
    }

    void restore_prev_state()
    {
        // Global scope is kept
        auto global_scope = scopes.front();
        scopes = std::move(switch_stack.back());
        scopes.front() = std::move(global_scope);
        switch_stack.pop_back();
    }
};

}
