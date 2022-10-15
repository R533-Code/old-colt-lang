#include <util/colt_pch.h>
#include <parsing/colt_lexer.h>
#include <ast/colt_ast.h>

using namespace colt;

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

    lang::COLTContext ctx;
    if (auto AST = lang::CreateAST({ buffer, WithNUL }, ctx))
    {
      io::PrintMessage("Parsed successfully!", buffer);
      if (is_a<lang::BinaryExpr*>(AST.get_value().expressions.get_back()))
        io::PrintMessage("Parsed a binary expression!");
    }
    else
    {
      io::PrintError("Parsed unsuccessfully!", buffer);
    }
  }  
}