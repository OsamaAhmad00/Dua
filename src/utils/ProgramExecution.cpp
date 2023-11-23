#include <boost/process.hpp> // This has to be at the top
#include <utils/ProgramExecution.h>
#include <string>
#include <vector>
#include <utils/ErrorReporting.h>
#include <thread>


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
        // This captures stdout only, ignoring stderr.
        bp::child c(path, args, bp::std_out > is, bp::std_err > bp::null);

        char buf;
        result.std_out.clear();
        while (c.running() && !is.eof()) {
            is.read(&buf, 1);
            result.std_out += buf;
        }
        // remove the extra repeated character at the end
        if (!result.std_out.empty())
            result.std_out.pop_back();

        c.wait();

        result.exit_code = c.exit_code();

        // FIXME remove this. Without this line, the deletion of the
        //  temporary test executables fails with an "access denied" message.
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    catch (std::exception& e) {
        report_internal_error(
                std::string("Encountered an error while executing the program output: ") + e.what());
    }

    return result;
}

}
