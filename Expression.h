#pragma once

#include <string>
#include <vector>
#include <memory>
#include <llvm/IR/Value.h>

enum class SExpressionType
{
    LIST,

    // Atom types
    STRING,
    SYMBOL,
    NUMBER,
};

struct Expression
{
    // FIXME as of now, a list, a string, and a number will be allocated for all types
    SExpressionType type;
    std::vector<Expression> list;
    std::string str;
    long long num;

    Expression(std::vector<Expression> list) : type(SExpressionType::LIST), list(std::move(list)) {}
    Expression(SExpressionType type, std::string str) : type(type), str(std::move(str)) {}
    Expression(long long num) : type(SExpressionType::NUMBER), num(num) {}
};