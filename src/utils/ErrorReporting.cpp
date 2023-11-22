#include <utils/ErrorReporting.h>
#include <utils/termcolor.hpp>

namespace dua
{

void report_error(const std::string& message)
{
    std::cerr << termcolor::red << "Error: " << termcolor::reset << message << '\n';
    throw std::runtime_error(message);
}

void report_internal_error(const std::string& message)
{
    std::cerr << termcolor::red << "Internal Error: " << termcolor::reset << message << '\n';
    throw std::runtime_error(message);
}

}
