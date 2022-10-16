#include <util/colt_pch.h>
#include <ast/colt_ast.h>

using namespace colt;
using namespace colt::lang;

void compile(StringView str) noexcept
{
  //Record beginning of compilation
  auto begin_time = std::chrono::steady_clock::now();

  //Compile
  COLTContext ctx;
  auto AST = CreateAST(str, ctx);

  //Record end of compilation
  io::PrintMessage("Finished compilation in {:.6}.",
    std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - begin_time));

  if (AST.is_value())
    io::PrintMessage("Compilation successful!");
  else
    io::PrintWarning("Compilation failed!");
}

void REPL() noexcept
{
  char buffer[2048];
  for (;;)
  {
    fgets(buffer, 2048, stdin);
    if (feof(stdin))
      break;    

    compile({ buffer, WithNUL });
  }
}

int main(int argc, const char** argv)
{
  //Populates GlobalArguments
  args::ParseArguments(argc, argv);

  if (args::GlobalArguments.file_in)
  {
    auto str = String::getFileContent(args::GlobalArguments.file_in);
    if (str.is_error())
      io::PrintError("Error reading file at path '{}'.", args::GlobalArguments.file_in);
    else
      compile(str.get_value());
  }
  else
  {
    REPL();
  }
}