cmake_minimum_required(VERSION 3.26)
project(Dua)

# Required for antlr4
set(CMAKE_CXX_STANDARD 17)

include_directories(${PROJECT_SOURCE_DIR}/include)

# Start of LLVM ---------
find_package(LLVM REQUIRED CONFIG)

include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

llvm_map_components_to_libnames(LLVM_LIBS support core irreader)
# End of LLVM -----------


# Start of antlr4 -------
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/antlr4/cmake)

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
set(ANTLR4_ZIP_REPOSITORY ${PROJECT_SOURCE_DIR}/antlr4/antlr4-4.13.1.zip)

# Located at antlr4/cmake
include(ExternalAntlr4Cpp)

include_directories(${ANTLR4_INCLUDE_DIRS})

# Set a variable pointing to the antlr tool that supports C++
# this is not required if the jar file can be found under PATH environment
set(ANTLR_EXECUTABLE ${PROJECT_SOURCE_DIR}/antlr4/antlr-4.13.1-complete.jar)

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
# End of antlr4 ---------

#

# Start of Boost --------
find_package(Boost COMPONENTS filesystem regex REQUIRED)

if(Boost_FOUND)
    message(STATUS "Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")
    message(STATUS "Boost_LIBRARIES: ${Boost_LIBRARIES}")
    message(STATUS "Boost_VERSION: ${Boost_VERSION}")
    include_directories(${Boost_INCLUDE_DIRS})
endif()
# End of Boost ----------

set(DUA_SOURCES
    ${ANTLR_DuaLexer_CXX_OUTPUTS}
    ${ANTLR_DuaParser_CXX_OUTPUTS}

    src/ModuleCompiler.cpp
    src/Preprocessor.cpp
    src/TypingSystem.cpp
    src/Value.cpp

    src/resolution/NameResolver.cpp
    src/resolution/FunctionNameResolver.cpp
    src/resolution/ClassResolver.cpp
    src/resolution/TemplatedNameResolver.cpp

    src/parsing/ParserAssistant.cpp
    src/parsing/ParserFacade.cpp

    src/utils/CodeGeneration.cpp
    src/utils/ErrorReporting.cpp
    src/utils/ProgramExecution.cpp
    src/utils/TextManipulation.cpp
    src/utils/VectorOperations.cpp

    src/AST/IndexingNode.cpp
    src/AST/AssignmentNode.cpp
    src/AST/ASTNode.cpp
    src/AST/BlockNode.cpp
    src/AST/CastExpressionNode.cpp
    src/AST/ClassDefinitionNode.cpp
    src/AST/ClassFieldDefinitionNode.cpp
    src/AST/ClassFieldNode.cpp
    src/AST/DereferenceNode.cpp
    src/AST/DoWhileNode.cpp
    src/AST/ForNode.cpp
    src/AST/FunctionCallNode.cpp
    src/AST/ExprFunctionCallNode.cpp
    src/AST/FunctionDefinitionNode.cpp
    src/AST/GlobalVariableDefinitionNode.cpp
    src/AST/IfNode.cpp
    src/AST/LoadedLValueNode.cpp
    src/AST/LocalVariableDefinitionNode.cpp
    src/AST/MallocNode.cpp
    src/AST/NumericalUnaryExpressionNodes.cpp
    src/AST/PostfixAdditionExpressionNode.cpp
    src/AST/PrefixAdditionExpressionNode.cpp
    src/AST/ReturnNode.cpp
    src/AST/StringValueNode.cpp
    src/AST/VariableNode.cpp
    src/AST/WhileNode.cpp
    src/AST/TypeNameNode.cpp
    src/AST/MethodCallNode.cpp
    src/AST/ClassIDNode.cpp
    src/AST/DynamicNameNode.cpp

    src/types/Type.cpp
    src/types/ArrayType.cpp
    src/types/PointerType.cpp
    src/types/ClassType.cpp
    src/types/FunctionType.cpp
    src/types/IdentifierType.cpp
    src/types/TypeOfType.cpp
    src/types/ReferenceType.cpp
)

set(DUA_PREREQUISITE_LIBS ${LLVM_LIBS} ${ANTLR4_LIBS} ${Boost_LIBRARIES})

add_executable(Dua
    main.cpp
    ${DUA_SOURCES}
)

target_link_libraries(Dua ${DUA_PREREQUISITE_LIBS})

# Testing related
enable_testing()
add_library(DUA_LIB ${DUA_SOURCES})
add_subdirectory(testing)