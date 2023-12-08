#include "FileTestCasesRunner.hpp"
#include "utils/termcolor.hpp"
#include <utils/ErrorReporting.hpp>
#include <Preprocessor.hpp>
#include <utils/TextManipulation.hpp>
#include <utils/CodeGeneration.hpp>
#include <utils/ProgramExecution.hpp>
#include <filesystem>
#include <gtest/gtest.h>


namespace dua
{

void FileTestCasesRunner::run()
{
    auto path = std::filesystem::weakly_canonical(TESTS_PATH + filename).string();
    auto tests_str = read_file(path);
    auto tests = split_cases(tests_str);
    try
    {
        int passed_cases = 0;
        for (size_t i = 0; i < tests.cases.size(); i++)
        {
            auto [header, body] = split_header_body(tests.cases[i]);
            auto name = extract_header_element(header, "Case");

            std::cerr << std::left << std::setw(60) << "Test " + std::to_string(i + 1) + ": " + name;
            std::cerr.flush();

            auto expected_exit_code_str = extract_header_element(header, "Returns");
            auto expected_output = extract_header_element(header, "Outputs");
            bool expected_output_is_empty = true;
            if (expected_output.size() >= 2) {
                expected_output_is_empty = false;
                expected_output = escape_characters(expected_output.substr(1, expected_output.size() - 2));
            } else if (!expected_output.empty()) {
                report_error(name + ": in test case " + std::to_string(i + 1) +
                             ", the expected output should be in-between \"\".");
            }
            bool should_panic = header_has_flag(header, "Panics");

            Preprocessor preprocessor;
            std::string code = tests.common + '\n' + body;
            try {
                code = preprocessor.process(path, code);
            } catch (std::exception &e) {
                std::cerr << "Panicked at the preprocessing stage\n";
                continue;
            }

            if (!expected_output.empty() && expected_output.back() != '\n') {
                // This is to match the output of the program execution, which
                //  will append a \n to the output if there isn't one.
                expected_output.push_back('\n');
            }

            auto temp_name = uuid();
            // Windows will complain if the extension is not .exe,
            //  and it doesn't hurt when run on Unix-based systems.
            auto exe_name = temp_name + +".exe";

            std::vector<std::string> n = {temp_name};
            std::vector<std::string> c = {code};
            std::vector<std::string> a = {"-o", exe_name};

            bool succeeded;

            if (should_panic) {
                succeeded = true;
                EXPECT_ANY_THROW(succeeded = !run_clang_on_llvm_ir(n, c, a));
                if (succeeded)
                    std::cerr << termcolor::green << "Exception thrown, Passed!" << termcolor::reset << '\n';
                passed_cases += succeeded;
                continue;
            } else {
                succeeded = false;
                EXPECT_NO_THROW(succeeded = run_clang_on_llvm_ir(n, c, a));
            }

            ProgramExecution execution;

            try {
                execution = execute_program(exe_name, args);
            } catch (...) {
                std::filesystem::remove(exe_name);
                std::cerr << "Panicked at the program execution stage\n";
                continue;
            }

            std::filesystem::remove(exe_name);

            if (!expected_output_is_empty) {
                EXPECT_EQ(expected_output, execution.std_out);
                if (expected_output != execution.std_out)
                    succeeded = false;
            }

            if (expected_exit_code_str.empty()) expected_exit_code_str = "0";
            int expected_exit_code;
            try {
                expected_exit_code = std::stoi(expected_exit_code_str);
            } catch (std::exception& e) {
                std::cerr << "Invalid expected return code: '" + expected_exit_code_str << "'\n";
                continue;
            }
            EXPECT_EQ((expected_exit_code % 256 + 256) % 256, (execution.exit_code % 256 + 256) % 256)
                                << "Actual exit code is " << execution.exit_code << '\n';
            if (((expected_exit_code % 256 + 256) % 256) != ((execution.exit_code % 256 + 256) % 256))
                succeeded = false;

            if (succeeded) std::cerr << termcolor::green << "Passed!" << termcolor::reset;
            std::cerr << '\n';
            passed_cases += succeeded;
        }

        std::cerr << "\n" << passed_cases << "/" << tests.cases.size() << " cases passed\n";

    } catch (std::exception& e) {
        report_internal_error(std::string("An exception threw with the message '")
                              + e.what() + "'\n");
        exit(-1);
    }
}

}