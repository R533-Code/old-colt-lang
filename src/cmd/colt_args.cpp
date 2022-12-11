/** @file colt_args.cpp
* Contains definition of functions declared in 'colt_args.h'.
*/

#include "colt_args.h"
#include "util/colt_print.h"

namespace colt::args
{    
  void ParseArguments(int argc, const char** argv) noexcept
  {
    for (size_t current_arg = 1; current_arg < static_cast<size_t>(argc); ++current_arg)
    {
      StringView arg_view = argv[current_arg];			

      if (arg_view[0] != '-')
      {				
        if (std::error_code code; !std::filesystem::exists(arg_view.get_data(), code))
        {
          io::PrintError("File at path '{}' does not exist!", arg_view);
          std::exit(1);
        }
        details::global_args.file_in = arg_view.get_data();
        continue;
      }
      if (arg_view.get_size() < 2)
        details::print_error_and_exit("Invalid argument '{}'!", arg_view);

      details::handle_arg(arg_view, argc, argv, current_arg);
    }
    if (details::global_args.opt_level == static_cast<gen::OptimizationLevel>(0))
      details::global_args.opt_level = gen::OptimizationLevel::O1;
  }

  namespace details
  {
    /*************************************
    * ARGUMENT CALLBACKS
    *************************************/

    void version_callback(int argc, const char** argv, size_t& current_arg) noexcept
    {
      if (current_arg != 1)
        print_error_and_exit("Invalid combination for argument '{}'!", argv[current_arg]);
      
      if (global_args.colored_output)
        io::Print(CONSOLE_COLOR_REVERSE "COLT" CONSOLE_COLOR_RESET CONSOLE_FOREGROUND_BRIGHT_MAGENTA " v{}" CONSOLE_COLOR_RESET " on {} ({})",
          COLT_VERSION_STRING, COLT_OS_STRING, COLT_CONFIG_STRING);
      else
        io::Print("COLT v{} on {} ({})", COLT_VERSION_STRING, COLT_OS_STRING, COLT_CONFIG_STRING);
      std::exit(0);
    }

    void help_callback(int argc, const char** argv, size_t& current_arg) noexcept
    {
      if (current_arg != 1 || argc != 3)
        print_error_and_exit("Invalid combination for argument '{}'!", argv[current_arg]);
      ++current_arg; //Advance to the next argument

      auto it = find_arg_in_predefined(argv[current_arg]);
      if (it == PredefinedArguments.end())
        print_error_and_exit("Unknown argument!");
      io::Print("{}", it->help);
      std::exit(0);
    }

    void enum_callback(int argc, const char** argv, size_t& current_arg) noexcept
    {
      if (current_arg != 1)
        print_error_and_exit("Invalid combination for argument '{}'!", argv[current_arg]);
      io::Print("List of valid commands:");
      for (const auto& argument : PredefinedArguments)
        io::Print("  --{}, -{}", argument.name, argument.abrv);
      std::exit(0);
    }

    void print_ir_callback(int argc, const char** argv, size_t& current_arg) noexcept
    {
      global_args.print_llvm_ir = true;
    }

    void no_color_callback(int argc, const char** argv, size_t& current_arg) noexcept
    {
      global_args.colored_output = false;
    }

    void no_error_callback(int argc, const char** argv, size_t& current_arg) noexcept
    {
      global_args.print_errors = false;
    }

    void no_warning_callback(int argc, const char** argv, size_t& current_arg) noexcept
    {
      global_args.print_warnings = false;
    }

    void no_message_callback(int argc, const char** argv, size_t& current_arg) noexcept
    {
      global_args.print_messages = false;
    }

    void out_callback(int argc, const char** argv, size_t& current_arg) noexcept
    {
      auto file = argv[++current_arg];
      if (!colt::isValidFileName({ file, std::strlen(file) }))
        print_error_and_exit("Path '{}' is invalid!", file);
      global_args.file_out = file;
    }

    void no_wait_callback(int argc, const char** argv, size_t& current_arg) noexcept
    {
      global_args.wait_for_user_input = false;
    }

    void o0_callback(int argc, const char** argv, size_t& current_arg) noexcept
    {
      if (global_args.opt_level != static_cast<gen::OptimizationLevel>(0))
        print_error_and_exit("Optimization level was already set!");
      global_args.opt_level = gen::OptimizationLevel::O0;
    }

    void o1_callback(int argc, const char** argv, size_t& current_arg) noexcept
    {
      if (global_args.opt_level != static_cast<gen::OptimizationLevel>(0))
        print_error_and_exit("Optimization level was already set!");
      global_args.opt_level = gen::OptimizationLevel::O1;
    }

    void o2_callback(int argc, const char** argv, size_t& current_arg) noexcept
    {
      if (global_args.opt_level != static_cast<gen::OptimizationLevel>(0))
        print_error_and_exit("Optimization level was already set!");
      global_args.opt_level = gen::OptimizationLevel::O2;
    }

    void o3_callback(int argc, const char** argv, size_t& current_arg) noexcept
    {
      if (global_args.opt_level != static_cast<gen::OptimizationLevel>(0))
        print_error_and_exit("Optimization level was already set!");
      global_args.opt_level = gen::OptimizationLevel::O3;
    }

    void os_callback(int argc, const char** argv, size_t& current_arg) noexcept
    {
      if (global_args.opt_level != static_cast<gen::OptimizationLevel>(0))
        print_error_and_exit("Optimization level was already set!");
      global_args.opt_level = gen::OptimizationLevel::Os;
    }

    void oz_callback(int argc, const char** argv, size_t& current_arg) noexcept
    {
      if (global_args.opt_level != static_cast<gen::OptimizationLevel>(0))
        print_error_and_exit("Optimization level was already set!");
      global_args.opt_level = gen::OptimizationLevel::Oz;
    }

    /*************************************
    * ARGUMENT HANDLING
    *************************************/
    
    void handle_arg(StringView arg_view, int argc, const char** argv, size_t& current_arg) noexcept
    {
      auto it = find_arg_in_predefined(arg_view);
      if (it == PredefinedArguments.end())
        print_error_and_exit("Unknown argument '{}'!", arg_view);
      else if (it->nb_of_args > static_cast<size_t>(argc) - current_arg - 1)
        print_error_and_exit("'{}' expects at least {} argument{}",
          arg_view, it->nb_of_args, it->nb_of_args == 1 ? "!" : "s!");
      else
        it->callback(argc, argv, current_arg);
    }
    
    auto find_arg_in_predefined(StringView arg_view) noexcept -> decltype(PredefinedArguments.end())
    {
      if (arg_view[0] == '-' && arg_view.get_size() > 1)
      {
        if (arg_view[1] == '-') // --<name>
        {
          arg_view.pop_front_n(2); //we skip '--'
          //Find using comparisons of '--name'
          return std::find_if(PredefinedArguments.begin(), PredefinedArguments.end(),
            [=](const Argument& val) { return val.name == arg_view; });
        }
        else // -<abrv>
        {
          arg_view.pop_front(); //we skip '-'
          //Find using comparisons of '-abrv'
          return std::find_if(PredefinedArguments.begin(), PredefinedArguments.end(),
            [=](const Argument& val) { return val.abrv == arg_view; });
        }
      }
      return PredefinedArguments.end();
    }
  }
}