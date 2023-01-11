#include <main_util.h> //include every colt related functionality

using namespace colt;

int main(int argc, const char** argv)
{
  //Populates the GlobalArguments
  args::ParseArguments(argc, argv);
  //Initialize code generators
  InitializeCOLT();

  if (args::GlobalArguments.file_in != nullptr)
    CompileFile(args::GlobalArguments.file_in);
  else
    REPL();

  if (args::GlobalArguments.wait_for_user_input)
    io::PressToContinue();
}