#include <util/colt_pch.h>
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
    gen::LLVMIRGenerator gen = { AST.get_value(), llvm::OptimizationLevel::O1};
  }
  else
    io::PrintWarning("Compilation failed with {} error{}", AST.get_error(), AST.get_error() == 1 ? "!" : "s!");
}

void REPL() noexcept
{
  char buffer[2048];
  for (;;)
  {
    if (fgets(buffer, 2048, stdin) == nullptr)
      break;

    compile({ buffer, WithNUL });
  }
}

int main(int argc, const char** argv)
{
  //Populates GlobalArguments
  args::ParseArguments(argc, argv);

  if (args::GlobalArguments.file_in != nullptr)
  {
    auto str = String::getFileContent(args::GlobalArguments.file_in);
    if (str.is_error())
      io::PrintError("Error reading file at path '{}'.", args::GlobalArguments.file_in);
    else
    {
      str.get_value().c_str(); //Appends a NUL terminator if needed
      compile(str.get_value());
    }
  }
  else
  {
    REPL();
  }
}