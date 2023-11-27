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
        // This captures stdout only, ignoring stderr.
        bp::child c(program, args, bp::std_out > is, bp::std_err > bp::null);

        std::string line;
        result.std_out.clear();
        // This will add a \n to the end of the output even if there wasn't one.
        while (c.running() && !is.eof() && std::getline(is, line)) {
            // FIXME this is to avoid the \r windows puts before the \n.
            //  This will produce wrong results if the text is intended
            //  to have a \r at the end.
            if (line.back() == '\r') line.pop_back();
            result.std_out += line + '\n';
        }

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
