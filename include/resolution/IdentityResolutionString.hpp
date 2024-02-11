#pragma once

#include <resolution/ResolutionString.hpp>

namespace dua
{

class IdentityResolutionString : public ResolutionString
{
public:

    std::string str;

    IdentityResolutionString(ModuleCompiler* compiler, std::string str) : str(std::move(str)) { this->compiler = compiler; }

    std::string resolve() override { return str; }
};

}