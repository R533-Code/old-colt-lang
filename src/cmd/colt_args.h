/** @file colt_args.h
* Contains Command Line argument parsing helpers.
* To parse the arguments, call ParseArguments({ argv, static_cast<size_t>(argc) }).
* This will populate the global argument holder.
* The colt::args namespace holds the GlobalArguments pointer which points to the parsed
* arguments. In case ParseArguments was not called, it contained the defaulted values of
* arguments.
*/

#ifndef HG_COLT_ARGS
#define HG_COLT_ARGS

#include <array>
#include <algorithm>
#include <string_view>
#include <filesystem>

#include <colt/String.h>

/// @brief Contains utilities for parsing command line arguments
namespace colt::args
{
	/// @brief Holds the result of parsing command line arguments
	struct Arguments
	{
		/// @brief The path to the file to compile
		const char* file_in = nullptr;
		/// @brief The path where to output the compiled file
		const char* file_out = nullptr;
		/// @brief If true, allows printing functions to use colored output
		bool colored_output = true;
		/// @brief If true, allows messages to be printed
		bool print_messages = true;
		/// @brief If true, allows warnings to be printed
		bool print_warnings = true;
		/// @brief If true, allows errors to be printed
		bool print_errors = true;
		/// @brief If true, prints informations about front-end allocations
		bool print_allocation_data = false;
	};

	/// @brief Parses the command line arguments, and stores them globally.
	/// This functions exits if the arguments are not valid.
	/// @param argc The argument count
	/// @param argv The argument values
	/// @return The result of the arguments
	void ParseArguments(int argc, const char** argv) noexcept;

	namespace details
	{
		/// @brief Global arguments that can be accessed by args::GlobalArguments
		inline Arguments global_args;

		/// @brief Specifies the arguments of an Argument callback
		using ARG_CALLBACK = void(*)(int, const char**, size_t&) noexcept;

		/// @brief Represents a command line argument
		struct Argument
		{
			/// @brief --name
			StringView name;
			/// @brief -abrv
			StringView abrv;
			/// @brief The -h [--name/-abrv]
			StringView help;
			/// @brief The number of expected arguments
			size_t nb_of_args;
			/// @brief The function to call when the Argument found was valid
			ARG_CALLBACK callback;
		};

		/// @brief Handles version argument
		/// @param args All the command line arguments
		/// @param current_arg The current argument number being parsed
		void version_callback(int argc, const char** argv, size_t& current_arg) noexcept;
		/// @brief Handles help argument
		/// @param args All the command line arguments
		/// @param current_arg The current argument number being parsed
		void help_callback(int argc, const char** argv, size_t& current_arg) noexcept;
		/// @brief Handles enum argument
		/// @param args All the command line arguments
		/// @param current_arg The current argument number being parsed
		void enum_callback(int argc, const char** argv, size_t& current_arg) noexcept;
		/// @brief Handles no-color argument
		/// @param args All the command line arguments
		/// @param current_arg The current argument number being parsed
		void no_color_callback(int argc, const char** argv, size_t& current_arg) noexcept;
		/// @brief Handles no-error argument
		/// @param args All the command line arguments
		/// @param current_arg The current argument number being parsed
		void no_error_callback(int argc, const char** argv, size_t& current_arg) noexcept;
		/// @brief Handles no-warn argument
		/// @param args All the command line arguments
		/// @param current_arg The current argument number being parsed
		void no_warning_callback(int argc, const char** argv, size_t& current_arg) noexcept;
		/// @brief Handles no-message argument
		/// @param args All the command line arguments
		/// @param current_arg The current argument number being parsed
		void no_message_callback(int argc, const char** argv, size_t& current_arg) noexcept;
		/// @brief Handles out argument
		/// @param args All the command line arguments
		/// @param current_arg The current argument number being parsed
		void out_callback(int argc, const char** argv, size_t& current_arg) noexcept;
		/// @brief Handles alloc-data argument
		/// @param args All the command line arguments
		/// @param current_arg The current argument number being parsed
		void alloc_data_callback(int argc, const char** argv, size_t& current_arg) noexcept;

		/// @brief Contains all predefined valid arguments
		constexpr std::array PredefinedArguments
		{
			Argument{ "version", "v", "Prints the version of the compiler.\nUse: --version/-v", 0, &version_callback},
			Argument{ "help", "h", "Prints the documentation of a command.\nUse: --help/-h <COMMAND>", 1, &help_callback},
			Argument{ "enum", "e", "Enumerates all possible commands.\nUse: --enum/-e <COMMAND>", 0, &enum_callback},
			Argument{ "no-color", "C", "Removes colored/highlighted outputs on the console.\nUse: --no-color/-C", 0, &no_color_callback},
			Argument{ "no-error", "E", "Removes error outputs.\nUse: --no-error/-E", 0, &no_error_callback},
			Argument{ "no-warn", "W", "Removes warning outputs.\nUse: --no-warn/-W", 0, &no_warning_callback},
			Argument{ "no-message", "M", "Removes message outputs.\nUse: --no-message/-M", 0, &no_message_callback},
			Argument{ "out", "o", "Specifies the output location.\nUse: --out/-o <PATH>", 1, &out_callback},
			Argument{ "alloc-data", "a", "Prints allocation informations of the front-end.\nUse: --alloc-data/-a <PATH>", 0, &alloc_data_callback}
		};

		/// @brief Handles an argument, searching for it and doing error handling
		/// @param arg_view The current argument to parse
		/// @param args The arguments to parse
		/// @param current_arg The number of the current argument
		void handle_arg(StringView arg_view, int argc, const char** argv, size_t& current_arg) noexcept;

		/// @brief Searches for an argument in predefined_args
		/// @param arg_view The argument to find
		/// @return Iterator to the argument or predefined_args.end() if not found
		auto find_arg_in_predefined(StringView arg_view) noexcept -> decltype(PredefinedArguments.end());		
	}

	/// @brief Global Arguments used throughout the Colt-Compiler
	inline const Arguments* const GlobalArguments = &details::global_args;

	namespace details
	{
		template<typename... Args>
		/// @brief Prints an error and exits
		[[noreturn]] void print_error_and_exit(fmt::format_string<Args...> fmt, Args&&... args) noexcept;

		template<typename... Args>
		void print_error_and_exit(fmt::format_string<Args...> fmt, Args && ...args) noexcept
		{
			if (args::GlobalArguments->colored_output)
				fmt::print(fg(fmt::color::red) | fmt::emphasis::bold, "Error: ");
			else
				fmt::print("Error: ");

			fmt::print(fmt, std::forward<Args>(args)...);
			std::fputc('\n', stdout);
			std::exit(1);
		}
	}
}

#endif //!HG_COLT_ARGS