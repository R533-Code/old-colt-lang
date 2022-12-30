/** @file main_util.h
* Contains helper functions that joins front-end and back-end, into
* simple function calls.
* This header imports all utilities provided by Colt.
*/

#ifndef COLT_MAIN_UTIL
#define COLT_MAIN_UTIL

#include <util/colt_config.h>

#if defined(COLT_MSVC) && defined(COLT_DEBUG)
  //FOR MEMORY LEAK DETECTION
  #define _CRTDBG_MAP_ALLOC
  #include <stdlib.h>
  #include <crtdbg.h>
#endif

#include <util/colt_pch.h>
#include <ast/colt_ast.h>

#ifndef COLT_NO_LLVM
  #include <code_gen/llvm_ir_gen.h>
  #include <interpreter/colt_JIT.h>
#endif //!COLT_NO_LLVM

namespace colt
{
  /// @brief Initializes the backend (code generator) of Colt
  void InitializeBackend() noexcept;

  /// @brief Enters REPL (Read Eval Print Loop) of Colt
  void REPL() noexcept;
  
  /// @brief Compiles a file, and depending on global arguments, uses the result.
  /// @param path The path of the file to compile
  void CompileFile(const char* path) noexcept;

  /// @brief Compiles a string, and depending on global arguments, uses the result.
  /// @param str The StringView to compile
  void CompileStr(StringView str) noexcept;

  /// @brief Compiles an Abstract Syntax Tree to IR, and depending on global arguments uses the result.
  /// @param ast The valid AST to compile
  void CompileAST(const lang::AST& ast) noexcept;

#ifndef COLT_NO_LLVM
  /// @brief Attempts to run the 'main' function from IR
  /// @param IR The IR to compile and in which to search for 'main' symbol
  void RunMain(gen::GeneratedIR&& IR, bool print = true) noexcept;
#endif //!COLT_NO_LLVM
}

#endif //!COLT_MAIN_UTIL