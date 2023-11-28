#pragma once

#include <unordered_map>
#include <string>
#include <vector>
#include <stdexcept>
#include <utils/ErrorReporting.h>

namespace dua
{

template<typename T>
class Scope
{

public:

    Scope(std::function<void(const T&)>* on_insertion_callback = nullptr,
          std::function<void(const T&)>* on_deletion_callback = nullptr)
          : on_insertion_callback(on_insertion_callback),
            on_deletion_callback(on_deletion_callback) {}

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

private:
    template <typename U> friend class SymbolTable;
    std::function<void(const T&)>* on_insertion_callback = nullptr;
    std::function<void(const T&)>* on_deletion_callback = nullptr;
    std::unordered_map<std::string, T> map;
};

template<typename T>
class SymbolTable
{
public:

    SymbolTable(std::function<void(const T&)>* on_insertion_callback = nullptr,
                std::function<void(const T&)>* on_deletion_callback = nullptr,
                bool apply_for_global_scope = false)
                : on_insertion_callback(on_insertion_callback),
                  on_deletion_callback(on_deletion_callback)
    {
        if (apply_for_global_scope)
            push_scope(Scope<T>(on_insertion_callback, on_deletion_callback));
        else
            push_scope();
    }

    size_t size() const { return scopes.size(); }

    SymbolTable& insert(const std::string& name, const T& t, int scope_num = 1) {
        if (scopes.empty()) {
            report_internal_error("There is no scope");
        }
        auto& scope = scopes[scopes.size() - scope_num];
        if (scope.on_insertion_callback != nullptr)
            (*scope.on_insertion_callback)(t);
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
        push_scope(Scope<T>(on_insertion_callback, on_deletion_callback));
        return *this;
    }

    Scope<T> pop_scope() {
        Scope<T> top = std::move(scopes.back());
        scopes.pop_back();
        if (top.on_deletion_callback != nullptr) {
            for (auto &element: top.map) {
                (*top.on_deletion_callback)(element.second);
            }
        }
        return top;
    }

    void set_insertion_callback(std::function<void(const T&)>* on_insertion_callback) {
        this->on_insertion_callback = on_insertion_callback;
    }

    void set_deletion_callback(std::function<void(const T&)>* on_deletion_callback) {
        this->on_deletion_callback = on_deletion_callback;
    }

private:
    std::function<void(const T&)>* on_insertion_callback = nullptr;
    std::function<void(const T&)>* on_deletion_callback = nullptr;
    std::vector<Scope<T>> scopes;
};

}
