#include <util/colt_pch.h>
#include <parsing/colt_lexer.h>
#include <ast/colt_expr.h>

using namespace colt;

int main(int argc, const char** argv)
{
  //Populates GlobalArguments
  args::ParseArguments(argc, argv);

  char buffer[2048];
  for (;;)
  {
    fgets(buffer, 2048, stdin);
    if (feof(stdin))
      break;

    lang::Lexer lexer = { StringView{ buffer, WithNUL } };
    lang::Token tkn = lexer.get_next_token();
    while (tkn != lang::TKN_EOF)
      tkn = lexer.get_next_token();
  }

  io::PrintMessage("Hello Colt!");
  io::PrintWarning("Hello Colt!");
  io::PrintError("Hello Colt!");
}