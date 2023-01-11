/** @file main_util.cpp
* Contains definition of functions declared in 'main_util.h'.
*/

#include "main_util.h"

using namespace colt::gen;
using namespace colt::lang;

namespace colt
{
  void CompileFile(const char* path) noexcept
  {
    auto str = String::getFileContent(path);
    if (str.is_error())
      io::PrintError("Error reading file at path '{}'.", path);
    else
    {
      str.get_value().c_str(); //Appends a NUL terminator if needed
      CompileStr(str.get_value());
    }
  }

  void InitializeCOLT() noexcept
  {
#if defined(COLT_MSVC) && defined(COLT_DEBUG)
    //Print memory leaks
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF /*| _CRTDBG_CHECK_ALWAYS_DF*/);
#endif
    colt::memory::RegisterOnNULLFn([]() noexcept {
      io::PrintFatal("Not enough memory to continue execution! Aborting...");
      });

#ifndef COLT_NO_LLVM
    //Initialize targets of current machine
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmParser();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetDisassembler();
#endif //!COLT_NO_LLVM
  }

  void REPL() noexcept
  {
    COLTContext ctx;
    AST ast = { ctx };

    for (;;)
    {
      ast.expressions.clear();
      io::Print<false>("{}>{} ", io::BrightCyanF, io::Reset);
      auto line = String::getLine();
      if (line.is_error())
        break;
      if (line->is_empty())
        continue;
      
      StringView str = *line;
      str.strip_spaces();
      if (!str.begins_with("fn") && !str.begins_with("var"))
      {
        auto to_cmp = String{
          "extern fn _ColtPrintbool(bool a)->void;\n"
          "extern fn _ColtPrinti8(i8 a)->void;\n"
          "extern fn _ColtPrinti16(i16 a)->void;\n"
          "extern fn _ColtPrinti32(i32 a)->void;\n"
          "extern fn _ColtPrinti64(i64 a)->void;\n"
          "extern fn _ColtPrintu8(u8 a)->void;\n"
          "extern fn _ColtPrintu16(u16 a)->void;\n"
          "extern fn _ColtPrintu32(u32 a)->void;\n"
          "extern fn _ColtPrintu64(u64 a)->void;\n"
          "extern fn _ColtPrintu8HEX(u8 a)->void;\n"
          "extern fn _ColtPrintu16HEX(u16 a)->void;\n"
          "extern fn _ColtPrintu32HEX(u32 a)->void;\n"
          "extern fn _ColtPrintu64HEX(u64 a)->void;\n"
          "extern fn _ColtPrintf32(float a)->void;\n"
          "extern fn _ColtPrintf64(double a)->void;\n"
          "extern fn _ColtPrintchar(char a)->void;\n"
          "extern fn _ColtPrintlstring(lstring a)->void;\n"
          "//BOOL OVERLOADS"
          "fn print(bool a)->void: _ColtPrintbool(a);\n"
          "//SIGNED INTS OVERLOADS"
          "fn print(i8 a)->void: _ColtPrinti8(a);\n"
          "fn print(i16 a)->void: _ColtPrinti16(a);\n"
          "fn print(i32 a)->void: _ColtPrinti32(a);\n"
          "fn print(i64 a)->void: _ColtPrinti64(a);\n"
          "//UNSIGNED INTS OVERLOADS"
          "fn print(u8 a)->void: _ColtPrintu8(a);\n"
          "fn print(u16 a)->void: _ColtPrintu16(a);\n"
          "fn print(u32 a)->void: _ColtPrintu32(a);\n"
          "fn print(u64 a)->void: _ColtPrintu64(a);\n"
          "//BYTES OVERLOADS"
          "fn print(BYTE a)->void: _ColtPrintu8HEX(a);\n"
          "fn print(WORD a)->void: _ColtPrintu16HEX(a);\n"
          "fn print(DWORD a)->void: _ColtPrintu32HEX(a);\n"
          "fn print(QWORD a)->void: _ColtPrintu64HEX(a);\n"
          "//FLOATING POINT OVERLOADS"
          "fn print(float a)->void: _ColtPrintf32(a);\n"
          "fn print(double a)->void: _ColtPrintf64(a);\n"
          "//STRINGS OVERLOADS"
          "fn print(char a)->void: _ColtPrintchar(a);\n"
          "fn print(lstring a)->void: _ColtPrintlstring(a);\n"
          "//EMPTY PARAMETERS"
          "fn print()->void: pass;\n"
          "fn main()->i64 { print(@line(1)\n"
        };
        to_cmp += str;
        to_cmp += "\n); }";
        to_cmp.c_str();
        if (CompileAndAdd(ctx.add_str(std::move(to_cmp)), ast))
        {
#ifndef COLT_NO_LLVM
          if (auto result = GenerateIR(ast); result.is_expected())
            RunMain(std::move(result.get_value()), false);
#endif //!COLT_NO_LLVM
        }
      }
      else
      {
        line->c_str();
        if (CompileAndAdd(ctx.add_str(std::move(line.get_value())), ast))
        {
#ifndef COLT_NO_LLVM
          if (auto result = GenerateIR(ast); result.is_expected())
            RunMain(std::move(result.get_value()), false);
#endif //!COLT_NO_LLVM
        }
      }
    }
  }

  void CompileStr(StringView str) noexcept
  {
    if (str.is_empty())
      return;

    //Record beginning of compilation
    auto begin_time = std::chrono::steady_clock::now();

    //Compile
    COLTContext ctx;
    auto AST = CreateAST(str, ctx);

    //Record end of compilation
    io::PrintMessage("Finished compilation in {:.6}.",
      std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - begin_time));

    if (AST.is_expected())
    {
      io::PrintMessage("Compilation successful!");
      CompileAST(AST.get_value());
    }
    else
      io::PrintWarning("Compilation failed with {} error{}", AST.get_error(), AST.get_error() == 1 ? "!" : "s!");
  }

  void CompileAST(const lang::AST& ast) noexcept
  {
#ifndef COLT_NO_LLVM
    auto IR = gen::GenerateIR(ast);
    if (IR.is_error())
    {
      io::PrintError("{}", IR.get_error());
      return;
    }

    //Optimize resulting IR
    IR->optimize(args::GlobalArguments.opt_level);

    if (args::GlobalArguments.print_llvm_ir) //Print IR
      IR->print_module(llvm::errs());
    if (args::GlobalArguments.file_out) //Write object file
    {
      if (auto result = IR->to_object_file(args::GlobalArguments.file_out); result.is_error())
        io::PrintError("{}", result.get_error());
      else
        io::PrintMessage("Successfully written object file '{}'!", args::GlobalArguments.file_out);
    }

    if (args::GlobalArguments.jit_run_main)
      RunMain(std::move(*IR));
#endif //!COLT_NO_LLVM
  }

#ifndef COLT_NO_LLVM
  void RunMain(gen::GeneratedIR&& IR, bool print) noexcept
  {
    if (auto JITError = gen::ColtJIT::Create(); !JITError)
    {
      io::PrintFatal("Could not initialize JIT compiler!");
      abort();
    }
    else
    {
      const auto& ColtJIT = std::move(*JITError);
      if (auto AddError = ColtJIT->addModule(std::move(IR)); AddError)
      {
        io::PrintFatal("Could not JIT compile the code!");
        abort();
      }
      else if (auto main = ColtJIT->lookup("main"))
      {
        if (print)
          io::PrintMessage("Running 'main' function...");
        
        auto main_fn = reinterpret_cast<i64(*)()>(main->getValue());
        i64 ret = main_fn();
        
        if (print)
          io::PrintMessage("'main' function returned '{}'!", ret);
      }
      else if (print)
        io::PrintWarning("'main' function was not found!");
    }
  }  
#endif //!COLT_NO_LLVM
}