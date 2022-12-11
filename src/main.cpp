#include <util/colt_pch.h>
#include <llvm/Support/TargetSelect.h>
#include <code_gen/llvm_ir_gen.h>

using namespace colt;
using namespace colt::lang;

void compile(StringView str) noexcept
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
    gen::LLVMIRGenerator gen = { AST.get_value(), llvm::OptimizationLevel::O1 };
    if (args::GlobalArguments.print_llvm_ir)
      gen.print_module();
    if (args::GlobalArguments.file_out)
      gen.to_object_file(args::GlobalArguments.file_out);
  }
  else
    io::PrintWarning("Compilation failed with {} error{}", AST.get_error(), AST.get_error() == 1 ? "!" : "s!");
}

void CompileFile(const char* path)
{
  auto str = String::getFileContent(path);
  if (str.is_error())
    io::PrintError("Error reading file at path '{}'.", path);
  else
  {
    str.get_value().c_str(); //Appends a NUL terminator if needed
    compile(str.get_value());
  }
}

void REPL() noexcept
{
  char buffer[2048];
  for (;;)
  {
    fputs(CONSOLE_FOREGROUND_BRIGHT_CYAN "> " CONSOLE_COLOR_RESET, stdout);
    if (fgets(buffer, 2048, stdin) == nullptr)
      break;

    compile({ buffer, WithNUL });
  }
}

int main(int argc, const char** argv)
{
  //Populates GlobalArguments
  args::ParseArguments(argc, argv);
  //Initialize LLVM specific functions
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmParser();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::InitializeNativeTargetDisassembler();

  if (args::GlobalArguments.file_in != nullptr)
    CompileFile(args::GlobalArguments.file_in);
  else
    REPL();
  io::PressToContinue();
}