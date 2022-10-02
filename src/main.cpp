#include <util/colt_pch.h>

using namespace colt;

int main(int argc, const char** argv)
{
  io::print_message("Hello Colt!");
  UniquePtr ptr = make_unique<int>(10);
}