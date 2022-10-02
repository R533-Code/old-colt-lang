#include <util/colt_pch.h>

using namespace colt;

class Base
{
public:
  enum type_of
  {
    BASE, DERIVED_A, DERIVED_B
  };

  static constexpr type_of classof_v = BASE;

  virtual type_of classof() noexcept { return BASE; }
};

class DerivedA
  : public Base
{
public:
  static constexpr type_of classof_v = DERIVED_A;
  
  type_of classof() noexcept override { return DERIVED_A; }
};

int main(int argc, const char** argv)
{
  //Populates GlobalArguments
  args::ParseArguments(argc, argv);

  Base* ptr = new DerivedA();
  if (is_a<DerivedA*>(ptr))
    io::print_message("ptr is DerivedA");

  io::print_message("Hello Colt!");
  io::print_warning("Hello Colt!");
  io::print_error("Hello Colt!");
}