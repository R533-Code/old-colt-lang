#include <main_util.h> //include every colt related functionality
#include <random> //for _ColtRand

using namespace colt;

COLT_EXPORT i64 _ColtRand(i64 a, i64 b)
{
  static std::mt19937 generator(std::random_device{}());
  std::uniform_int_distribution<i64> distr(a, b);
  return distr(generator);
}

COLT_EXPORT void _ColtPrinti64(i64 a)
{
  io::Print("{}", a);
}

COLT_EXPORT PTR<const char> _ColtHelloWorld()
{
  return "Hello Colt!";
}

COLT_EXPORT PTR<const char> _ColtPrintfi64()
{
  return "i64 %lld\n";
}

int main(int argc, const char** argv)
{
  //Populates the GlobalArguments
  args::ParseArguments(argc, argv);
  //Initialize code generators
  InitializeBackend();

  if (args::GlobalArguments.file_in != nullptr)
    CompileFile(args::GlobalArguments.file_in);
  else
    REPL();

  if (args::GlobalArguments.wait_for_user_input)
    io::PressToContinue();
}