#include <boost/process.hpp> // This has to be at the top
#include <utils/ProgramExecution.hpp>
#include <string>
#include <vector>
#include <utils/ErrorReporting.hpp>
#include <thread>


namespace dua
{

namespace bp = boost::process;

std::string capture_output(bp::ipstream& is, bool& within_limit)
{
    std::string result, line;
    while (!is.eof() && std::getline(is, line) && within_limit)
    {
        // FIXME this is to avoid the \r windows puts before the \n.
        //  This will produce wrong results if the text is intended
        //  to have a \r at the end.
        // This will add a \n to the end of the output even if there wasn't one.
        if (line.back() == '\r') line.pop_back();
        result += line + '\n';
    }
    return result;
}

ProgramExecution execute_program(const std::string& program, const std::vector<std::string>& args, long long time_limit)
{
    ProgramExecution result { "THE PROGRAM DIDN'T EXECUTE!!", -989898989};

    bp::ipstream is; //reading pipe-stream
    // This captures stdout only, ignoring stderr.
    bp::child c(program, args, bp::std_out > is, bp::std_err > bp::null);

    std::string line;
    result.std_out.clear();

    bool within_limit = true;
    auto output = std::async(&capture_output, std::ref(is), std::ref(within_limit));
    auto start = std::chrono::system_clock::now();

    output.wait_for(std::chrono::milliseconds(time_limit));
    auto current = std::chrono::system_clock::now();
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds >(current - start).count();
    within_limit = milliseconds < time_limit;
    c.terminate();

    // FIXME remove this. Without this line, the deletion of the
    //  temporary test executables fails with an "access denied" message.
    std::this_thread::sleep_for(std::chrono::milliseconds(5));

    result.std_out = output.get();
    result.exit_code = c.exit_code();

    if (!within_limit)
        throw TLEException();

    return result;
}

}
