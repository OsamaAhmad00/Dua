#include <boost/process.hpp> // This has to be at the top
#include <utils/ProgramExecution.h>
#include <string>
#include <vector>
#include <iostream>
#include <utils/ErrorReporting.h>


namespace dua
{

namespace bp = boost::process;

ProgramExecution execute_program(const std::string& program, const std::vector<std::string>& args)
{
    ProgramExecution result { "THE PROGRAM DIDN'T EXECUTE!!", -989898989};

    try
    {
        bp::ipstream is; //reading pipe-stream
        auto path = bp::search_path(program);
        bp::child c(path, args, bp::std_out > is, bp::std_err > bp::null);

        std::string line;
        result.std_out.clear();
        while (c.running() && std::getline(is, line))
            result.std_out += line + '\n';

        c.wait();

        result.exit_code = c.exit_code();
    }
    catch (std::exception& e) {
        report_internal_error(
                std::string("Encountered an error while executing the program output: ") + e.what());
    }

    return result;
}

}
