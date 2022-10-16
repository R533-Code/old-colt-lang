#include <util/colt_pch.h>
#include <ast/colt_ast.h>

using namespace colt;
using namespace colt::lang;

int main(int argc, const char** argv)
{
  //Populates GlobalArguments
  args::ParseArguments(argc, argv);

  io::PrintMessage("Hello Colt!");
  io::PrintWarning("Hello Colt!");
  io::PrintError("Hello Colt!");

  char buffer[2048];
  for (;;)
  {
    fgets(buffer, 2048, stdin);
    if (feof(stdin))
      break;

    //Record beginning of compilation
    auto begin_time = std::chrono::steady_clock::now();

    //Compile
    COLTContext ctx;
    auto AST = CreateAST({ buffer, WithNUL }, ctx);
    
    //Record end of compilation
    io::PrintMessage("Finished compilation in {:.6}.",
      std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - begin_time));

    if (AST)
    {
      io::PrintMessage("Parsed successfully!", buffer);
      if (is_a<BinaryExpr>(AST.get_value().expressions.get_back()))
        io::PrintMessage("Parsed a binary expression!");
    }
    else
    {
      io::PrintError("Parsed unsuccessfully!", buffer);
    }
  }
}