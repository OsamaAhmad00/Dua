#include <utils/ErrorReporting.hpp>
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

void report_warning(const std::string& message)
{
    std::cerr << termcolor::yellow << "Warning: " << termcolor::reset << message << '\n';
}

}
