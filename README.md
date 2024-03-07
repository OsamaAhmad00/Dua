<div align="center"> <img width="256" src="dua.svg"> </div>

# Dua

The Dua Compiler is a cross-platform compiler for the Dua language, written in C++, leveraging the power of the LLVM compiler infrastructure.

## Platform and Architecture Support

Thanks to the LLVM backend, the Dua compiler is equipped with a broad range of platform and architecture support. It seamlessly targets various platforms including but not limited to:

- Windows
- Linux
- macOS
- Android
- iOS

In addition, it supports a multitude of architectures, encompassing:

- x86/x86-64
- WASM32/64
- ARM/ARM64
- RISCV32/64
- MIPS/MIPS64
- Thumb

The compiler is capable of handling both little and big endian, wherever applicable.


## Simple Example

This example demonstrates the use of templated classes, inheritance, method overriding, and output streams in a simple Dua program.

```
class Box<T>
{
    T value;

    constructor(T value) : value(value) { }

    void show() {
        out << "Value: " << value << "\n";
    }
}

class IntBox : Box<int>
{
    constructor(int i) : Super(i) { }

    // Overridden method
    void show() {
        out << "Integer Value: " << value << "\n";
    }
}

int main()
{
    Box<String> sbox("Hello World");
    IntBox ibox(5);
	
    sbox.show();
    ibox.show();
	
    /*
    * Output:
    * Value: Hello World
    * Integer Value: 5
    */
}
```


## Language Features

### Object-Oriented Programming
The Dua language supports the core concepts of object-oriented programming, including classes, inheritance, method overriding, and method overloading. Examples can be found in the following files:
- [class-construction.dua](examples/class-construction.dua)
- [class-definition.dua](examples/class-definition.dua)
- [inheritance.dua](examples/inheritance.dua)

### Type Inference
The Dua language is capable of inferring types, which simplifies code and improves readability. See examples in [type-inference.dua](examples/type-inference.dua).

### Polymorphism
The Dua language supports both static-time (through templates) and runtime polymorphism, managing overloaded functions, templated entities, and operator overloading. Examples can be found in the following files:
- [generic-classes.dua](examples/generic-classes.dua)
- [generic-functions.dua](examples/generic-functions.dua)
- [functions.dua](examples/functions.dua)
- [function-overloading.dua](examples/function-overloading.dua)

### Meta-Programming
The Dua language supports meta-programming. Examples can be found in the following files:
- [typename-operator.dua](examples/typename-operator.dua)
- [typeof-operator.dua](examples/typeof-operator.dua)
- [noref-operator.dua](examples/noref-operator.dua)
- [istype-operator.dua](examples/istype-operator.dua)

### Operator Overloading
The Dua language allows for operator overloading, enabling custom behavior for operators. Examples can be found in the following files:
- [infix-operators.dua](examples/infix-operators.dua)
- [postfix-operators.dua](examples/postfix-operators.dua)

### Type-Aliasing
The Dua language supports type-aliasing. You can find examples in [type-alias.dua](examples/type-alias.dua).

### Low-Level Manipulation
The Dua language provides the ability to manipulate low-level details by offering multiple helpful operators, including `sizeof`, `offsetof`, `dynamicname`, `typename`, `_set_vtable`, and more. Examples can be found in the following files:
- [sizeof-operator.dua](examples/sizeof-operator.dua)
- [offsetof-operator.dua](examples/offsetof-operator.dua)
- [dynamicname-operator.dua](examples/dynamicname-operator.dua)
- [typename-operator.dua](examples/typename-operator.dua)
- [set-vtable-operator.dua](examples/set-vtable-operator.dua)

### Standard Library
The Dua language includes a standard library with essential classes and algorithms, including vectors, strings, priority queues, I/O streams, and more. Examples can be found in the following files:
- [vector.dua](examples/vector.dua)
- [string.dua](examples/string.dua)
- [priority-queue.dua](examples/priority-queue.dua)
- [algorithms.dua](examples/algorithms.dua)

The [examples](examples) folder contains over 450 examples demonstrating language usage.


## Example Projects

The [projects](projects) folder includes a collection of sample projects showcasing the capabilities of the Dua language. Notable examples include:

- [Sudoku Solver](projects/SudokuSolver): An efficient program to solve Sudoku puzzles.
- [Graph Shortest Path Finder](projects/Dijkstra): A tool that computes the shortest path in a graph using Dijkstra's shortest-path algorithm

These projects serve as practical references for understanding the syntax and features of the Dua language.


## Compilation Features

The Dua Compiler comes with the ability to generate and link against both static and dynamic libraries. It has the versatility to produce LLVM IR, assembly, object files, or executables.

By default, the Dua compiler links against two libraries: libc and libdua. If you wish to avoid linking against these libraries, you can use the `-no-stdlib` option for libc and `-no-libdua` for libdua.

The components of libdua are globally accessible throughout the program. However, to utilize libc, a declaration of the function intended for use must be written.


## Installation and Usage

#### Prerequisites
Ensure that `clang` version 15.0 or higher is installed and accessible via one of the following commands: `clang`, `clang-15`, `clang-16`, or `clang-17`.

#### Installing Dua Compiler
Download and install the Dua compiler installer from the releases page. The compiler requires `clang` to function properly.

#### Compiling a Basic Dua Programs
To compile a Dua program, it's as simple as the following command:
```
Dua example.dua -o example.exe
```

The Dua compiler is built upon Clang, ensuring compatibility with all post Intermediate Representation (IR) generation Clang arguments. Consequently, standard Clang commands are applicable, such as `-c` to generate object files and `-S -emit-llvm` for LLVM IR output, among others.

#### Manual Installation
After building, place the `libdua` library file in a standard library directory (e.g., `/lib` or `/usr/lib` on Linux) or in the same directory as the Dua compiler executable. For convenience, add the path of the Dua compiler executable to the `PATH` environment variable or move it to a directory included in the `PATH` (e.g., `/bin` or `/usr/bin` on Linux).


## Testing

### Adding Test Cases

To introduce new test cases, follow these steps:

1. Create a new test source file within the `testing/tests` folder and name the file according to the test case, appending the `.cpp` extension.

2. Utilize the `define_test` macro in `testing\tests\CMakeLists.txt`, passing it the test name to be registered.

### Utilizing the Existing Testing Framework

The used testing framework is built on GoogleTest. To leverage it:

- Employ the `FileTestCasesRunner` class located in the `testing\tests` folder.
- The `FileTestCasesRunner` requires the test file name as an argument, and expects to find the test file at the `examples` folder.

This approach serves dual purposes:

- It provides a repository of test cases.
- It offers examples demonstrating language usage.

### Test File Structure

When writing a test file, consider the following structure:

- **Common Section**: This is the initial portion of the file up to the first test case. It's prepended to each case automatically.
- **Cases**: Each case is divided into a header and a body.
  - **Header**: Composed of single-line comments (`//`), detailing the test specifications.
  - **Body**: Contains the actual test code.

### Test Case Header

Test case headers consist of following:

- **Case Name**: Denoted by `// Case <name>`. This is the only mandatory element, and must be the header's first line.

Other optional header elements are:
- **Expected Exit Code**: `// Returns <exit_code>`.
- **Expected Stdout**: `// Outputs <stdout_output>`. Stderr is not captured.
- **Time Limit**: `// Time Limit <time_limit_in_ms>`.
- **Compile-Time Failure**: `// Panics`. This indicates that the test should fail during compilation.
- **Exceeds Time Limit**: `// Exceeds time limit`. This flags tests expected to run longer than the specified limit.

To ensure that there are no runtime failures, assert that the program exits with the intended exit code (usually 0).

**Note**: The header is not case-sensitive. You may use any preferred casing.

Test case examples can be found at the [examples](examples) folder


## Project Folder Structure

The project is organized as follows:

- **antlr4**: This folder houses essential files that enable the linkage and execution of Antlr4 for parsing the source files.
- **examples**: A collection of various language feature examples, which also serve as tests to ensure the compiler's accuracy and reliability.
- **grammar**: Here, you'll find the grammar files that Antlr4 utilizes to produce the parser.
- **include**: Contains the header files that are included across the project.
- **lib**: The source code for `libdua`, the core library of the Dua language, resides here.
- **projects**: This folder is dedicated to showcasing example projects written in the Dua language.
- **scripts**: Scripts that aid in the building and installation process of the compiler for both Windows and Linux systems can be found here.
- **src**: All the source files that constitute the compiler are located within this folder.
- **testing**: Comprises test files and the necessary tools for conducting thorough testing of the compiler.


## Prerequisites for the Project

Before you begin, ensure you have the following prerequisites installed and set up:

- **Java**: Required to run the `Antlr4` jar file. The necessary `Antlr4` files can be found in the project's `Antlr4` folder.
- **LLVM**: This must be installed on your system. Please follow the installation guide appropriate for your operating system.
- **Boost**: This C++ library needs to be installed on your system. Installation instructions are available on the Boost official website.
- **GoogleTest**: For unit testing, simply clone the project repository along with the submodules to include GoogleTest.

Please refer to the installation guides for each prerequisite for detailed instructions.


## Todo

Here are some of the possible improvements to the language and the compiler:

  - Support default parameter values
  - Support casting operator
  - Support more operators (prefix, infix, postfix)
  - Support unsigned integers (trivial)
  - Support access modifiers for classes
  - Support exceptions and stack unwinding
  - Better error reporting
  - Syntax highlighting 
  - Don't create a vtable if it's not needed and save the pointer in each object
  - Better names for templated classes and functions when exposed in a message
  - Store the addresses in the symbol table in the memory_location field instead of in the loaded_value
  - Unify the use of the `loaded_value` and `memory_location` for the `dua::Value` struct in all functions instead of having some functions expecting the address in the `loaded_value` field, and other in the `memory_location` field.
  - Support compiling using multiple threads
  - Instead of having the type deduction logic (the `ASTNode::get_type` method), and the node evaluation logic (the `ASTNode::eval` method) separate, which may introduce inconsistency bugs, introduce a "dry-run" option in the eval method, that performs a dry-run, computing the desired node result type without evaluating the node. 
  - Source map generation and debugging support
  - Support the const qualifier
  - Support moving ownership (move constructor and move copy constructor)
  - Support nested classes and nested functions
  - Support function static variables, taking synchronization between multiple threads into consideration
  - Support static methods and static fields
  - Implement sophisticated imports and import resolution
  - Better command line interface (i.e. support the `Dua *.dua` command)
  - Restructure the project dependencies (includes and cmake dependencies), and linking options for different components (parser, compiler, tests)
  - Detect invalid circular dependencies in class creation and provide meaningful error messages for them
  - Update the CFG to continue parsing in presence of errors
  - Report all errors instead of stopping at the first one
  
