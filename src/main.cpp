#include <main_util.h> //include every colt related functionality
#include <interpreter/qword_op.h>
#include <random> //for _ColtRand
#include <iostream>

using namespace colt;

COLT_EXPORT i64 _ColtRand(i64 a, i64 b)
{
  static std::mt19937 generator(std::random_device{}());
  std::uniform_int_distribution<i64> distr(a, b);
  return distr(generator);
}

COLT_EXPORT void _ColtPrinti8(i8 a)           { io::Print("{}", a); }
COLT_EXPORT void _ColtPrinti16(i16 a)         { io::Print("{}", a); }
COLT_EXPORT void _ColtPrinti32(i32 a)         { io::Print("{}", a); }
COLT_EXPORT void _ColtPrinti64(i64 a)         { io::Print("{}", a); }
COLT_EXPORT void _ColtPrintu8(u8 a)           { io::Print("{}", a); }
COLT_EXPORT void _ColtPrintu16(u16 a)         { io::Print("{}", a); }
COLT_EXPORT void _ColtPrintu32(u32 a)         { io::Print("{}", a); }
COLT_EXPORT void _ColtPrintu64(u64 a)         { io::Print("{}", a); }
COLT_EXPORT void _ColtPrintbool(bool a)       { io::Print("{}", a); }
COLT_EXPORT void _ColtPrintf32(f32 a)         { io::Print("{}", a); }
COLT_EXPORT void _ColtPrintf64(f64 a)         { io::Print("{}", a); }
COLT_EXPORT void _ColtPrintchar(char a)       { io::Print("{}", a); }
COLT_EXPORT void _ColtPrintlstring(lstring a) { io::Print("{}", a); }

int main(int argc, const char** argv)
{
  io::PrintFatal("Not enough memory to continue execution! Aborting...");
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