#pragma once

#include <string>
#include <vector>

enum class SExpressionType
{
    LIST,

    // Atom types
    IDENTIFIER,
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

    Expression() : type(SExpressionType::LIST) {}
    Expression(std::vector<Expression> list) : type(SExpressionType::LIST), list(std::move(list)) {}
    Expression(SExpressionType type, std::string str) : type(type), str(std::move(str)) {}
    Expression(long long num) : type(SExpressionType::NUMBER), num(num) {}
};