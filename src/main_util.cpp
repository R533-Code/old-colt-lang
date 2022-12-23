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

  void InitializeBackend() noexcept
  {
    //Initialize targets of current machine
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmParser();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetDisassembler();
  }

  void REPL() noexcept
  {
    for (;;)
    {
      io::Print<false>("{}>{} ", io::BrightCyanF, io::Reset);
      auto line = String::getLine(WithNUL);
      if (line.is_error())
        break;

      CompileStr(line.get_value());
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
  }

  void RunMain(gen::GeneratedIR&& IR) noexcept
  {
    if (auto JITError = gen::ColtJIT::Create(); !JITError)
      io::PrintError("Could not create JIT compiler!");
    else
    {
      auto ColtJIT = JITError->get();
      if (auto AddError = ColtJIT->addModule(std::move(IR)); AddError)
        io::PrintError("Could not add module!");
      else if (auto main = ColtJIT->lookup("main"))
      {
        auto main_fn = reinterpret_cast<i64(*)()>(main->getValue());
        io::PrintMessage("Main function returned '{}'!", main_fn());
      }
      else
        io::PrintWarning("Main function was not found!");
    }
  }  
}