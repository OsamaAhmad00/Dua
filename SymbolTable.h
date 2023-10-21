#pragma once

#include <unordered_map>
#include <string>
#include <vector>
#include <stdexcept>

template<typename T>
class Scope
{
public:
    Scope& insert(const std::string& name, const T& t) {
        if (contains(name)) {
            throw std::runtime_error("The symbol " + name + " is already defined");
        }
        map[name] = t;
        return *this;
    }

    const T& get(const std::string& name) {
        if (!contains(name)) {
            throw std::runtime_error("The symbol " + name + " is not defined");
        }
        return map[name];
    }

    bool contains(const std::string& name) {
        return map.find(name) != map.end();
    }

private:
    std::unordered_map<std::string, T> map;
};

template<typename Local_T, typename Global_T>
class SymbolTable
{
public:
    SymbolTable& insert(const std::string& name, const Local_T& t, int scope_num = 1) {
        if (scopes.empty()) {
            throw std::runtime_error("There is no local scope");
        }
        scopes[scopes.size() - scope_num].insert(name, t);
        return *this;
    }

    SymbolTable& insert_global(const std::string& name, const Global_T& t) {
        global_scope.insert(name, t);
        return *this;
    }

    const Local_T& get(const std::string& name, int scope_num = 1) {
        if (scopes.empty()) {
            throw std::runtime_error("There is no local scope");
        }
        for (int i = scopes.size() - scope_num; i >= 1; i--) {
            if (scopes[i].contains(name)) {
                return scopes[i].get(name);
            }
        }
        return scopes[0].get(name);  // will throw an exception if not present here either.
    }

    const Global_T& get_global(const std::string& name) {
        return global_scope.get(name);
    }

    bool contains(const std::string& name) {
        for (const Scope<Local_T>& scope : scopes)
            if (scope.contains(name))
                return true;
        return false;
    }

    bool contains_global(const std::string& name) {
        return global_scope.contains(name);
    }

    SymbolTable& push_scope(Scope<Local_T> scope) {
        scopes.push_back(std::move(scope));
        return *this;
    }

    SymbolTable& push_scope() {
        push_scope(Scope<Local_T>());
        return *this;
    }

    Scope<Local_T> pop_scope() {
        Scope<Local_T> top = scopes.back();
        scopes.pop_back();
        return top;
    }

private:
    std::vector<Scope<Local_T>> scopes;
    Scope<Global_T> global_scope;
};