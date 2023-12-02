#include "FileTestCasesRunner.h"
#include "utils/ErrorReporting.h"
#include <utils/TextManipulation.h>
#include <utils/CodeGeneration.h>
#include <utils/ProgramExecution.h>
#include <filesystem>
#include <gtest/gtest.h>

namespace dua
{

void FileTestCasesRunner::run()
{
    auto path = std::filesystem::weakly_canonical(TESTS_PATH + filename).string();
    auto tests_str = read_file(path);
    auto tests = split_cases(tests_str);
    for (size_t i = 0; i < tests.cases.size(); i++)
    {
        auto [header, body] = split_header_body(tests.cases[i]);
        auto name = extract_header_element(header, "Case");
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
        auto code = tests.common + '\n' + body;

        if (!expected_output.empty() && expected_output.back() != '\n') {
            // This is to match the output of the program execution, which
            //  will append a \n to the output if there isn't one.
            expected_output.push_back('\n');
        }

        auto temp_name = uuid();
        // Windows will complain if the extension is not .exe,
        //  and it doesn't hurt when run on Unix-based systems.
        auto exe_name = temp_name +  + ".exe";

        std::string case_description = "The wrong test case is test case number "
                                       + std::to_string(i + 1) + ", with the name of '" + name + "'";

        try {
            std::vector<std::string> n = { temp_name };
            std::vector<std::string> c = { code };
            std::vector<std::string> a = { "-o", exe_name };
            if (should_panic) {
                EXPECT_ANY_THROW(run_clang_on_llvm_ir(n, c, a)) << case_description;
                continue;
            } else run_clang_on_llvm_ir(n, c, a);
        } catch (...) {
            exit(-1);
        }

        ProgramExecution execution;

        try {
            execution = execute_program(exe_name, args);
        } catch (...) {
            std::filesystem::remove(exe_name);
            exit(-1);
        }

        std::filesystem::remove(exe_name);

        if (!expected_exit_code_str.empty()) {
            int expected_exit_code = std::stoi(expected_exit_code_str);
            EXPECT_EQ((expected_exit_code % 256 + 256) % 256, (execution.exit_code % 256 + 256) % 256) << case_description;
        }

        if (!expected_output_is_empty) {
            EXPECT_EQ(expected_output, execution.std_out) << case_description;
        }

    }
}

}