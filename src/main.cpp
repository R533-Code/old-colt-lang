//include every colt related functionality:
#include <main_util.h>

using namespace colt;

int main(int argc, const char** argv)
{
  //Initialize code generators
  InitializeCOLT();
  //Populates the GlobalArguments
  args::ParseArguments(argc, argv);

  //Compile a file or enter REPL
  if (args::GlobalArguments.file_in != nullptr)
    CompileFile(args::GlobalArguments.file_in);
  else
    REPL();

  if (args::GlobalArguments.wait_for_user_input)
    io::PressToContinue();
}