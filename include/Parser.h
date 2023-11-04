#pragma once

#include <Expression.h>

class Parser
{
public:
    Expression parse(const std::string& str) const;
};