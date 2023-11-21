<p align="center" style="text-align: center;"> <img width="256" src="dua.svg"> </p>

# Dua

The Dua language compiler. Written in C++, and is based on LLVM, Antlr4, Boost libraries, and the GoogleTest framework

## Notable compiler functionalities

- Compiling to multiple architectures (such as x86, MIPS, RISC-V, and even PIC microcontrollers)
- Compiling to multiple OSs (such as Windows, Linux, and MacOSX)
- Generating and linking against both static and dynamic libraries
- Generating LLVM IR
- Generating assembly
- Generating object files
- Generating executables

---

## Examples on the compilation process

```bash
# Basic compilation:
dua main.dua

# Generate LLVM IR:
dua main.dua -S -emit-llvm

#Generate assembly:
dua main.dua -S

# Generate an object file:
dua main.dua -c

# Cross-compilation:
# For more information on targete triples, consult: https://clang.llvm.org/docs/CrossCompilation.html#target-triple
dua main.dua --target=x86_64-pc-windows-msvc
```

The compiler is based on clang (LLVM), which means that post-IR-generation options can be passed to the compiler.

---

The compiler links with `libc` automatically, which means that any `libc` function can be called within the Dua code.
You just need to declare the function before usage.

## Example of using LibC

```
int printf(i8* message, ...);

int main()
{
    printf("Hello, world!");
    return 0;
}
```

---

## Basic language syntax

For a detailed examination of the syntax, you can take a look at the `grammar` folder.

### Primitive data types

- `i64` / `long`         = 64-bit integer
- `i32` / `int`          = 32-bit integer
- `i16` / `short`        = 16-bit integer
- `i8` / `byte` / `bool` = 8-bit integer
- `f64` / `double`        = 64-bit floating point number
- `f32` / `float`         = 64-bit floating point number
- `ArrType[ArrSize]`     = An array with type `ArrType` and size `ArrSize`
- `PtrType*`             = A pointer type to a `PtrType` type
- `void`                 = The void type

  Further examples of the language are in the `examples` folder.

### Literals

- Integer literals:

  - Bases: Integer literals can be in one of the following bases:
    - Hexadecimal: Prefixed with `0x`. Example: `0x0123`
    - Decimal    : The default. No prefix.
    - Octal      : Prefixed with `0`. Example: `0123`
    - Binary     : Prefixed with `0b`. Example: `0b010101`
  - Width: Depending on the suffix, literals will have different bit-width (and will be casted as appropriate depending on the context).
    - 64-bits: The default bit-width for integers
    - 32-bits: 32-bit integers are suffixed with `I` (int).   Example: `123I`
    - 16-bits: 16-bit integers are suffixed with `S` (short). Example: `123S`
    - 8-bits : 8-bit  integers are suffixed with `T` (tiny).  Example: `123T`
- Floating-point literals:

  - 64-bits: The default bit-width for floating-point numbers
  - 32-bits: 32-bit floating-point numbers are suffixed with `F` (float). Example: `123.456F`
- String literals: String literals are put between double quotes. Example: `"Hello, world!"`
- Other: Other literals include `null`, `true`, and `false`.

### Variables:

Variables are defined as follows: `type name = value;`. For declarations, just omit the initialization. Global variables must be initialized.

### Comments:

- Single-line comments: Starts with `//` and ends by the end of the line.
- Multi-line comments: Starts with `/*` and ends with `*/`.

### Functions: Functions can be in declared/defined in the global scope only.

You can set the function to have variable-argument parameters by adding `...` at the end of the parameter list.

functions can have different types of bodies:

- Block body
- Expression body

For declarations, just omit the body, and add a `;` at the end.

Examples:

```
// Declaration
void func(int y);

// Vararg function declaration
void printf(i8* message, ...);

// Block-body
int add(int i, int j) { return i + j; }

// Expression body
int mult(int i, int j) = i * j;
```

### Expressions

Expressions don't end with `;`. If they did, they'll be considered an expression statement, and unless they have a side effect (such as the compound assignment operators), they will most likely be eliminated.

For expressions, we'll construct examples directly.

- If Expressions:

  ```
  // If expressions must have an else branch. 

  int min = if (x < y) x else y;

  int result = if (x > 50) 1 else if (x == 50) 0 else -1;
  ```
- The Ternary Operator:

  ```
  int min = (x < y) ? x : y;
  ```
- When Expressions:

  ```
  // Just like if expressions, when expressions must have an else branch.
  int min = when {
      x < y -> x,
      else  -> y
  };

  int result = when {
      x > 50  -> 1,
      x == 50 -> 0,
      else    -> -1
  };
  ```
- Cast Expressions:

  ```
      int* null_pointer = (int*)0;

      // Here, the casting is not necessary. It'll happen implicitly.
      long extended = (long)123T;

      // Same as above
      short truncated = 123I;
  ```
- Block Expressions: Block expressions end with an expression (no `;` at the end), and evaluate to that expression

  ```
  int value = { int x = get_x(); int y = get_y(); if (x < y) x else y };

  // result = 3
  int result = { print(x); 3 };
  ```
- Address Of and Dereferencing:

  ```
  int x = 5;
  int* ptr = &x;
  int** ptrptr = &ptr;

  // x = 3
  *ptr = 3;

  // x = 10
  ***&ptrptr = 10;
  ```
- Other: Other supported expressions include:

  - `()`: Parenthesized expression
  - `+`: Addition
  - `-`: Subtraction
  - `*`: Multiplication
  - `/`: Division
  - `%`: Mod
  - `++`: Pre/Post-Increment
  - `--`: Pre/Post-Decrement
  - `-`: Unary Minus (Negation)
  - `+`: Unary Plus  (Identity)
  - `!`: Not
  - `~`: Bitwise Complement
  - `<<`: Left Shift
  - `>>`: Right Shift
  - `>>>`: Arithmetic Right Shift
  - `<`: Less Than
  - `>`: Greater Than
  - `<=`: Less Than or Equal
  - `>=`: Greater Than or Equal
  - `==`: Equal
  - `!=`: Not Equal
  - `&`: Bitwise And
  - `^`: Xor
  - `|`: Bitwise Or
  - `&&`: Logical And
  - `||`: Logical Or
  - `=`: Equals
  - `+=`: Compound Assignment (addition)
  - `-=`: Compound Assignment (subtraction)
  - `*=`: Compound Assignment (multiplication)
  - `/=`: Compound Assignment (division)
  - `%=`: Compound Assignment (mod)
  - `<<=`: Compound Assignment (left shift)
  - `>>=`: Compound Assignment (right shift)
  - `>>>=`: Compound Assignment (arithmetic right shift)
  - `&=`: Compound Assignment (bitwise and)
  - `^=`: Compound Assignment (xor)
  - `|=`: Compound Assignment (bitwise or)

  Note that the logical `&&` and `||` exhibit the "short-circuiting" behaviour. Which means that later elements of an and or or expression won't get evaluated in case the result is already determined.
  This is helpful in checking for nullable types for example. Example:

  ```
  // Here, ptr won't be dereferenced if it's null
  if (ptr != null && *ptr == 5) ...;
  ```

### Statements

Just like expressions, we'll proceed with examples.

- If Statements:

  ```
  if (x < y)
      value = x;
  else
      value = y;

  if (x < 5) {
      value = 1;
  } else if (x == 5) {
      value = 0;
  } else {
      value = -1;
  }
  ```
- For Loops:

  ```
  for (int i = 0; i < n; i++) {
      if (...) continue;
      ...
      if (...) break;
  }

  // The condition and the update sections are expressions, which
  //  means that you can use block expressions in them.
  for (int i = 0, int j = 4; { int x = i * j - 3; x < 10 }; { i++; j++ })
  {
      ...
  }

  for (int i = 0; i < n; i++)
      print(i);

  // Sections can be empty.
  // Infinite loop
  for (;;);
  ```
- While Loops:

  ```
  while (x < y)
  {
      x++;
  }

  // Infinite loop
  while ();

  while (x < y)
      x++;
  ```
- Do-While Loops:

  ```
  do {
      ...
  } while (x < y);

  do print(x) while (x++ < y);

  // Infinite loop
  do; while();
  ```
- Other: Other statements include empty statements (just `;`), expression statements (expression followed by `;`), and block statements.

---

## Basic Language Semantics

It's worth Noting that blocks (expressions or statements) introduce a new scope, in which name shadowing applies.

This applies to blocks used in if, when, while, ... expressions/statements.

Example:

```
int x = 12;

int func()
{
    // Prints 12
    print(x);
}

int main()
{
    int x = 3;
  
    {
        int x = 5;
        // Prints 5
        print(x);
    }
  
    // Prints 3
    print(x);
}
```

---

## Project Folder Structure

The project is organized as follows:

- `antlr4`: Contains the files needed to be able to link and run Antlr4 to parse the source files
- `examples`: Contains examples of different language features. Also used as tests to assure compiler correctness
- `grammar`: Contains the grammar files used by Antlr4 to generate the parser
- `include`: Contains the include headers
  - `AST`: Contains classes representing AST nodes, which also hold the code generation logic
  - `parsing`: Contains helper classes related to parsing
  - `types`: Contains classes representing types
  - `utils`: Contains util functions such as the `termcolor` library, or `clang` related functions for final executable generation.
- `src`: Contains all source files
- `testing`: Contains the test files along with the tools needed for testing.
  - To add new test cases, you can add a new test source file in `testing/tests`, and use the `define_test` macro in `testing\tests\CMakeLists.txt` to include the new test. Note that the source file name should be the same as the test name, plus the `.cpp` extension.

## Project Prerequisites

- `Antlr4`: The needed files are in the `Antlr4` folder, but you need to have `Java` installed in order for the jar file to run.
- `LLVM`
- `Boost`

## Todo

Here are some of the possible improvements to the language and the compiler:

- OOP support
- Support multi-dimensional arrays
- Improve error messages
- Incorporate multithreading into LLVM IR generation
- Support higher-order functions
- Write more examples
