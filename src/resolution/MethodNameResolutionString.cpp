#include <resolution/MethodNameResolutionString.hpp>
#include <types/Type.hpp>

namespace dua
{

MethodNameResolutionString::MethodNameResolutionString(ModuleCompiler* compiler, const Type* class_type, std::string method)
    : class_type(class_type), method(std::move(method))
{
    this->compiler = compiler;
}

std::string MethodNameResolutionString::resolve()
{
    return class_type->to_string() + "." + method;
}

}