#include <util/colt_pch.h>

using namespace colt;

int main(int argc, const char** argv)
{
  //Populates GlobalArguments
  args::ParseArguments(argc, argv);
  
  io::print_message("Hello Colt!");
}