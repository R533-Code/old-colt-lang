#include <util/colt_pch.h>
#include <parsing/colt_lexer.h>
#include <ast/colt_expr.h>

using namespace colt;

int main(int argc, const char** argv)
{
  //Populates GlobalArguments
  args::ParseArguments(argc, argv);

  io::PrintMessage("Hello Colt!");
  io::PrintWarning("Hello Colt!");
  io::PrintError("Hello Colt!");
}