#include <boost/process.hpp> // This has to be at the top
#include <utils/ProgramExecution.h>
#include <string>
#include <vector>
#include <iostream>
#include <utils/ErrorReporting.h>

namespace bp = boost::process;


int get_exit_code(const std::string& program, const std::vector<std::string>& args)
{
    int result = -989898989;
    try
    {
        bp::child c(bp::search_path(program), args, bp::std_out > bp::null, bp::std_err > bp::null);
        c.wait();
        result = c.exit_code();
    }
    catch (std::exception& e) {
        std::cerr << "Encountered an error running program: " << e.what() << '\n';
    }
    return result;
}

std::string get_stdout(const std::string& program, const std::vector<std::string>& args)
{
    std::string result;

    try
    {
        bp::ipstream is; //reading pipe-stream
        bp::child c(bp::search_path(program), args, bp::std_out > is, bp::std_err > bp::null);

        std::string line;
        while (c.running() && std::getline(is, line))
            result += line + '\n';

        c.wait();
    }
    catch (std::exception& e) {
        std::cerr << "Encountered an error while capturing program output: " << e.what() << '\n';
    }

    return result;
}
