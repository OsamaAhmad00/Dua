cmake_minimum_required(VERSION 3.26)
project(Tests)

set(CMAKE_CXX_STANDARD 17)

set(TEST_LIBS gtest_main DUA_LIB ${DUA_PREREQUISITE_LIBS})

macro (define_test name)
    add_executable(${name} ${name}.cpp FileTestCasesRunner.cpp)
    target_link_libraries(${name} ${TEST_LIBS})
    add_test(NAME ${name} COMMAND ${name})
endmacro ()

define_test(WhenExpressions)
define_test(IfExpressions)
define_test(IfStatements)
define_test(Functions)
