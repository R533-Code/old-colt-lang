#ifndef HG_COLT_ARGS_V2
#define HG_COLT_ARGS_V2

#include "colt_args_parser.h"

#ifndef COLT_NO_LLVM
  //for the target triple
  #include <llvm/Config/llvm-config.h>
#endif

namespace colt::args
{  
  template<bool positional = false, typename T>
  void parse(T* write_result, StringView to_parse, StringView arg_name) noexcept
  {
    if (to_parse.is_empty())
    {
      if constexpr (std::is_same_v<std::decay_t<T>, bool>)
      {
        *write_result = true;
        return;
      }
      else
      {
        io::PrintError("Argument '{}' expects a value!", arg_name);
        std::exit(1);
      }
    }

    if (!parser<std::decay_t<T>, positional>{}(to_parse, write_result))
    {
      io::PrintError("Invalid value for argument '{}'!", arg_name);
      std::exit(1);
    }
  }

  //HELPERS FOR COMMANDS STARTING WITH "-"
#define GET_NAME(a, num, v, name, d) StringView{ name },
#define DECLARE_VAR(name, num, init, n, d) decltype(init) name = init;
#define HELP_STR(n, num, i, name, desc) "  -" name ":\n     " desc "\n\n"
#define GEN_CASE(a, num, v, name, desc) \
  if (name[0] == argv[i][1]) { \
      if (StringView{ argv[i] + 1 }.begins_with(name)) { \
          if constexpr (num != 0) { \
          if (num > argc - i - 1) { \
            io::PrintError("Argument '" name "' expects " #num " arguments!"); \
            std::exit(1); \
          } }\
          if constexpr (num == 1) {\
            parse<true>(&a, argv[++i], name); continue; }\
          else { \
            parse<false>(&a, argv[i] + sizeof(name), name); continue; }\
        } }

#define GEN_CASE_ALIAS(a, name) \
  if (name[0] == argv[i][1]) { \
      if (StringView{ argv[i] + 1 }.begins_with(name)) { parse(&a, argv[i] + sizeof(name), name); continue; } }

  //POSITIONAL ARGUMENTS

#define HELP_POS(n, t, name) "<" name "> "
#define DECLARE_VAR_POS(name, t, n) decltype(t) name = t;
#define GEN_BOOL(name, t, n) bool COLT_CONCAT(name, BOOL) = false;
#define IF_POS(name, type, n) \
  if (COLT_CONCAT(name, BOOL) == false) {\
    parse<true>(&name, argv[i], HELP_POS(name, type, n)); \
    COLT_CONCAT(name, BOOL) = true;\
    continue; \
  }

#define DECLARE_ARGS(COMMANDS, ALIAS, POSITIONALS) \
  void PrintHelp() noexcept {\
    io::Print( \
    "The COLT compiler and interpreter.\n\n" \
    "USAGE:  colt [options] " \
    POSITIONALS(HELP_POS) \
    "\n   or:  colt [options]\n\nOPTIONS:\n" \
    "  -help:\n     Display available options.\n\n" \
    "  -v:\n     Display compiler version informations.\n\n" \
      COMMANDS(HELP_STR) \
    ); \
  } \
  constexpr auto NameTable = sort(std::array{ COMMANDS(GET_NAME) }); \
  static_assert(details::is_unique(NameTable), "All arguments name should be unique!"); \
  constexpr auto MaxNameSize = details::max_name_size(NameTable); \
  COMMANDS(DECLARE_VAR) \
  POSITIONALS(DECLARE_VAR_POS)\
  void ParseArguments2(int argc, const char** argv) noexcept { \
    bool is_only_positional = false; \
    for (size_t i = 1; i < argc; i++) \
    { \
      if (StringView{ argv[i] } == "--") { \
        is_only_positional = true; continue; \
      } \
      if (argv[i][0] != '-' || is_only_positional) { \
        POSITIONALS(GEN_BOOL)\
        POSITIONALS(IF_POS)\
      } \
      if (StringView{ argv[i] } == "-help") \
      { \
        PrintHelp(); \
        continue; \
      } \
      if (StringView{ argv[i] } == "-v") \
      { \
        io::Print("{} v{} on {} ({})", \
        io::ColorEachStrChar{ "COLT " }, COLT_VERSION_STRING, COLT_OS_STRING, COLT_CONFIG_STRING); \
        continue; \
      } \
      COMMANDS(GEN_CASE) \
      ALIAS(GEN_CASE_ALIAS) \
      io::PrintError("Unknown argument '{}'! Try: 'colt -help'", argv[i]);\
      std::exit(1); \
    } \
  }

#ifdef COLT_NO_LLVM
  #define COLT_DEFAULT_TARGET "no-target"
#else
  #define COLT_DEFAULT_TARGET LLVM_DEFAULT_TARGET_TRIPLE
#endif

#define COLT_BUILTIN_COMMANDS(X) \
  X(NoColor,       0, false, "no-color", "Deactivates colored output.") \
  X(PrintLLVMIR,   0, false, "print-ir", "Prints LLVM IR.") \
  X(NoError,       0, false, "no-error", "Deactivates logging of compilation errors.") \
  X(NoWarning,     0, false, "no-warn", "Deactivates logging of compilation warnings.") \
  X(NoMessage,     0, false, "no-message", "Deactivates logging of compilation messages.") \
  X(RunMain,       0, false, "run-main", "Run the 'main' function inside the compiler if it exists.") \
  X(NoWait,        0, false, "no-wait", "Specifies that the compiler should exit without user input.") \
  X(FileOut,       1, (lstring)nullptr, "o", "Place the output into <file>.") \
  X(TargetMachine, 1, (lstring)COLT_DEFAULT_TARGET, "target", "Chooses the target for which to compile.")

#define COLT_ALIAS_COMMANDS(X) \
  X(NoColor,      "C") \
  X(PrintLLVMIR,  "ir") \
  X(NoError,      "E") \
  X(NoWarning,    "W") \
  X(NoMessage,    "M") \
  X(RunMain,      "r")

#define COLT_POSITIONAL_ARGS(X) \
  X(FileIn, (lstring)nullptr, "input file")
  //ADD ALIAS
  //ADD CHECK FOR UNIQUE WITH ALIAS
  //ADD SUPPORT FOR ARGS FOR OPTIONS
  //ADD BOOLEAN FOR POSITIONAL OR NOT PARSER
  DECLARE_ARGS(
    COLT_BUILTIN_COMMANDS,
    COLT_ALIAS_COMMANDS,
    COLT_POSITIONAL_ARGS
  );
}

#endif //!HG_COLT_ARGS_V2