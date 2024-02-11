#pragma once

#include <string>

namespace dua
{

class ModuleCompiler;

class ResolutionString
{
protected:

    ModuleCompiler* compiler = nullptr;

public:

    virtual std::string resolve() = 0;

    virtual ~ResolutionString() = default;
};

}