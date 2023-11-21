#pragma once

#include <utils/termcolor.hpp>

inline void report_error(const std::string& message)
{
    std::cerr << termcolor::red << "Error: " << termcolor::reset << message << '\n';
    throw std::runtime_error(message);
}

inline void report_internal_error(const std::string& message)
{
    std::cerr << termcolor::red << "Internal Error: " << termcolor::reset << message << '\n';
    throw std::runtime_error(message);
}