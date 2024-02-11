#pragma once

#include <resolution/ResolutionString.hpp>

namespace dua
{

class Type;

class MethodNameResolutionString : public ResolutionString
{
    const Type* class_type;
    std::string method;

public:

    MethodNameResolutionString(ModuleCompiler* compiler, const Type* class_type, std::string method);

    std::string resolve() override;
};

}