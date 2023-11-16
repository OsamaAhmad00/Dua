cmake_minimum_required(VERSION 3.26)
project(Dua)

# Required for Antlr4
set(CMAKE_CXX_STANDARD 17)

include_directories(${PROJECT_SOURCE_DIR}/include)

# Start of LLVM ---------
find_package(LLVM REQUIRED CONFIG)

include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

llvm_map_components_to_libnames(LLVM_LIBS support core irreader)
# End of LLVM -----------


# Start of Antlr4 -------
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/Antlr4/cmake)

# Required if linking to static library
add_definitions(-DANTLR4CPP_STATIC)

# Using /MD flag for antlr4_runtime (for Visual C++ compilers only)
set(ANTLR4_WITH_STATIC_CRT OFF)

# Specify the version of the antlr4 library needed for this project.
# By default the latest version of antlr4 will be used. You can specify a
# specific, stable version by setting a repository tag value or a link
# to a zip file containing the libary source.
set(ANTLR4_TAG 4.13.0)
# You can put a URL to the zip file here as well.
set(ANTLR4_ZIP_REPOSITORY ${PROJECT_SOURCE_DIR}/Antlr4/antlr4-4.13.1.zip)

# Located at Antlr4/cmake
include(ExternalAntlr4Cpp)

include_directories(${ANTLR4_INCLUDE_DIRS})

# Set a variable pointing to the antlr tool that supports C++
# this is not required if the jar file can be found under PATH environment
set(ANTLR_EXECUTABLE ${PROJECT_SOURCE_DIR}/Antlr4/antlr-4.13.1-complete.jar)

find_package(ANTLR REQUIRED)

# Add the lexer and the parser to the build dependencies.
antlr_target(DuaLexer ${PROJECT_SOURCE_DIR}/grammar/DuaLexer.g4 LEXER
        PACKAGE dua)  # namespace = dua
antlr_target(DuaParser ${PROJECT_SOURCE_DIR}/grammar/DuaParser.g4 PARSER
        PACKAGE dua
        DEPENDS_ANTLR DuaLexer
        COMPILE_FLAGS -lib ${ANTLR_DuaLexer_OUTPUT_DIR})  # namespace = dua

include_directories(${ANTLR_DuaLexer_OUTPUT_DIR})
include_directories(${ANTLR_DuaParser_OUTPUT_DIR})

set(ANTLR4_LIBS antlr4_static)
# End of Antlr4 ---------

FILE(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/src/*)

add_executable(Dua
    main.cpp
    ${SOURCES}
    ${ANTLR_DuaLexer_CXX_OUTPUTS}
    ${ANTLR_DuaParser_CXX_OUTPUTS}
)

target_link_libraries(Dua ${LLVM_LIBS} ${ANTLR4_LIBS})
