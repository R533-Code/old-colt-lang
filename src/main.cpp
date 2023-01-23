//include every colt related functionality:
#include <main_util.h>

using namespace colt;

int main(int argc, const char** argv)
{
  //Initialize code generators
  InitializeCOLT();
  //Populates the global arguments
  args::ParseArguments(argc, argv);

  //Compile a file or enter REPL
  if (args::FileIn != nullptr)
    CompileFile(args::FileIn);
  else
    REPL();

  if (!args::NoWait)
    io::PressToContinue();
}