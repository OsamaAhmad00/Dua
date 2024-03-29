#pragma once

#include <unordered_map>
#include <string>
#include <vector>
#include <stdexcept>
#include <utils/ErrorReporting.hpp>

namespace dua
{

template<typename T>
static std::string to_string(const T& t) {
    return std::to_string(t);
}

template<>
std::string to_string<std::string>(const std::string& str) {
    return str;
}

template<typename T, typename Key = std::string>
struct ScopeEntry
{
    Key symbol;
    T value;
};

template<typename T, typename Key = std::string>
struct Scope
{
    ModuleCompiler* compiler;
    // A vector is used here because the number
    //  of elements is usually small to an extent
    //  that the overhead of an std::map is bigger
    //  than just performing a linear search.
    // Also, to not store additional information to
    //  retain the orders of insertion, which will
    //  be needed for example when destructing a scope.
    std::vector<ScopeEntry<T, Key>> map;
    std::vector<ScopeEntry<T, Key>> moved_symbols;

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

    size_t insertion_order(const Key& symbol) const
    {
        for (size_t i = 0; i < map.size(); i++)
            if (map[i].symbol == symbol)
                return i;
        return -1;
    }

    T& get(const Key& symbol) {
        return map[get_valid_index(symbol)].value;
    }

    Scope& insert(const Key& symbol, const T& t)
    {
        if (contains(symbol)) {
            report_error("The symbol " + to_string(symbol) + " is already defined", compiler);
        }
        for (size_t i = 0; i < moved_symbols.size(); i++) {
            if (moved_symbols[i].symbol == symbol) {
                moved_symbols.erase(moved_symbols.begin() + i);
                break;
            }
        }
        map.push_back({symbol, t});
        return *this;
    }

    void erase(const Key& symbol) {
        map.erase(map.begin() + get_valid_index(symbol));
    }

    void move_erase(const Key& symbol)
    {
        auto index = get_valid_index(symbol);
        auto entry = std::move(map[index]);
        map.erase(map.begin() + index);
        for (size_t i = 0; i < moved_symbols.size(); i++) {
            if (moved_symbols[i].symbol == entry.symbol) {
                moved_symbols[i] = std::move(entry);
                return;
            }
        }
        moved_symbols.push_back(std::move(entry));
    }

    bool contains(const Key& symbol) const {
        return insertion_order(symbol) != (size_t)-1;
    }

    bool is_move_erased(const Key& symbol) const
    {
        for (size_t i = 0; i < moved_symbols.size(); i++) {
            if (moved_symbols[i].symbol == symbol) {
                return true;
            }
        }
        return false;
    }

private:

    size_t get_valid_index(const Key& symbol) const {
        size_t index = insertion_order(symbol);
        if (index == (size_t)-1)
            report_error("The symbol " + to_string(symbol) + " is not defined", compiler);
        return index;
    }

};

template<typename T, typename Key = std::string>
struct SymbolTable
{
    ModuleCompiler* compiler;
    std::vector<Scope<T, Key>> scopes;
    // Used when switching to a temporary state in which
    //  all scopes but the global one are discarded.
    std::vector<std::vector<Scope<T, Key>>> switch_stack;

    SymbolTable(ModuleCompiler* compiler) : compiler(compiler) { push_scope(); }

    size_t size() const { return scopes.size(); }

    SymbolTable& insert(const Key& name, const T& t, int scope_num = 1)
    {
        if (scopes.empty()) {
            report_internal_error("There is no scope", compiler);
        }
        auto& scope = scopes[scopes.size() - scope_num];
        scope.insert(name, t);
        return *this;
    }

    SymbolTable& insert_global(const Key& name, const T& t) {
        insert(name, t, size());
        return *this;
    }

    T& get(const Key& name, bool include_global = true, int scope_num = 1)
    {
        if (scopes.empty()) {
            report_internal_error("There is no scope", compiler);
        }

        bool is_moved_in_higher_scope = false;

        // Iterate over all but the last scope (taking include_global into consideration)
        for (int i = scopes.size() - scope_num; i >= 1 + !include_global; i--) {
            if (scopes[i].contains(name))
            {
                if (is_moved_in_higher_scope) {
                    report_warning("The variable " + to_string(name) +
                       " is moved in earlier scope, and the symbol " + to_string(name) +
                       " has been resolved to a variable with the same name but in an outer scope",
                       compiler
                    );
                }
                return scopes[i].get(name);
            }

            is_moved_in_higher_scope |= scopes[i].is_move_erased(name);
        }

        if (!scopes[!include_global].contains(name) && is_moved_in_higher_scope)
            report_error("The variable " + to_string(name) + " is moved and can't be referenced unless it's redefined again", compiler);

        return scopes[!include_global].get(name);
    }

    T& get_global(const Key& name) {
        return get(name, true, size());
    }

    bool contains(const Key& name, bool include_global = true, int scope_num = 1)
    {
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

    bool is_move_erased(const Key& name)
    {
        for (size_t i = size() - 1; i != (size_t)-1; i--)
            if (scopes[i].is_move_erased(name))
                return true;
        return false;
    }

    bool contains_global(const Key& name) {
        return contains(name, true, size());
    }

    SymbolTable& push_scope(Scope<T, Key> scope) {
        scopes.push_back(std::move(scope));
        return *this;
    }

    SymbolTable& push_scope() {
        push_scope(Scope<T, Key>(compiler));
        return *this;
    }

    Scope<T, Key> pop_scope() {
        Scope<T, Key> top = std::move(scopes.back());
        scopes.pop_back();
        return top;
    }

    void keep_only_last_n_scopes(size_t n, bool include_global_scope = true)
    {
        if ((n + include_global_scope) > scopes.size())
            report_internal_error("Can't keep " + std::to_string(n + include_global_scope) + " scopes. There are only " + std::to_string(scopes.size()) + " scopes", compiler);

        switch_stack.push_back(std::move(scopes));

        scopes.resize(n + include_global_scope, Scope<T, Key>(compiler));

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

    void remove_last_occurrence_of(const Key& name, bool panic_if_not_found = true)
    {
        for (size_t i = size() - 1; i != size_t(-1); i--) {
            if (scopes[i].contains(name)) {
                scopes[i].erase(name);
                return;
            }
        }

        if (panic_if_not_found)
            report_internal_error("The symbol " + to_string(name) + " is not present in the symbol table");
    }

    void move_last_occurrence_of(const Key& name, bool panic_if_not_found = true)
    {
        for (size_t i = size() - 1; i != size_t(-1); i--) {
            if (scopes[i].contains(name)) {
                scopes[i].move_erase(name);
                return;
            }
        }

        if (panic_if_not_found)
            report_internal_error("The symbol " + to_string(name) + " is not present in the symbol table");
    }
};

}
