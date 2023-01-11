//Include every colt related functionality:
#include <main_util.h>

using namespace colt;

int main(int argc, const char** argv)
{
  //Populates the GlobalArguments
  args::ParseArguments(argc, argv);
  //Initialize code generators
  InitializeCOLT();

  //Compile a file or enter REPL
  if (args::GlobalArguments.file_in != nullptr)
    CompileFile(args::GlobalArguments.file_in);
  else
    REPL();

  if (args::GlobalArguments.wait_for_user_input)
    io::PressToContinue();
}