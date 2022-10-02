/** @file colt_print.h
* Contains utilities for printing to the console.
* These utilities might seem redundant looking at colt_log.h.
* The difference is that logging utilities can be completely removed from a build.
*/

#ifndef HG_COLT_PRINT
#define HG_COLT_PRINT

#include <fmt/core.h>

/// @brief Contains utilities for printing to console
namespace colt::io
{
	template<bool active = true, typename... Args>
	/// @brief Prints to the standard output
	/// @tparam ...Args Pack of types to format
	/// @param fmt The format string, using {fmt} syntax
	/// @param ...args The arguments to format
	constexpr void print(fmt::format_string<Args...> fmt, Args&&... args);

	template<bool active = true, typename... Args>
	/// @brief Prints to the standard output
	/// @tparam ...Args Pack of types to format
	/// @param fmt The format string, using {fmt} syntax
	/// @param ...args The arguments to format
	constexpr void print_message(fmt::format_string<Args...> fmt, Args&&... args);

	template<bool active = true, typename... Args>
	/// @brief Prints to the standard output
	/// @tparam ...Args Pack of types to format
	/// @param fmt The format string, using {fmt} syntax
	/// @param ...args The arguments to format
	constexpr void print_warning(fmt::format_string<Args...> fmt, Args&&... args);

	template<bool active = true, typename... Args>
	/// @brief Prints to the standard output
	/// @tparam ...Args Pack of types to format
	/// @param fmt The format string, using {fmt} syntax
	/// @param ...args The arguments to format
	constexpr void print_error(fmt::format_string<Args...> fmt, Args&&... args);

	/// @brief Used to escape a character being printed.
	/// Example: print("{}", EscapeChar{'\n'}) -> '\n'
	struct EscapeChar
	{
		/// @brief The char to escape
		char chr;
	};

	template<bool active, typename... Args>
	constexpr void print(fmt::format_string<Args...> fmt, Args && ...args)
	{
		if constexpr (active)
		{
			fmt::print(fmt, std::forward<Args>(args)...);
			std::fputc('\n', stdout);
		}
	}

	template<bool active, typename... Args>
	constexpr void print_message(fmt::format_string<Args...> fmt, Args && ...args)
	{
		if constexpr (active)
		{
			/*if (args::GlobalArguments->colored_output)
				fmt::print(fg(fmt::color::cornflower_blue) | fmt::emphasis::bold, "Message: ");
			else*/
				fmt::print("Message: ");

			fmt::print(fmt, std::forward<Args>(args)...);
			std::fputc('\n', stdout);
		}
	}

	template<bool active, typename... Args>
	constexpr void print_warning(fmt::format_string<Args...> fmt, Args && ...args)
	{
		if constexpr (active)
		{
			/*if (args::GlobalArguments->colored_output)
				fmt::print(fg(fmt::color::yellow) | fmt::emphasis::bold, "Warning: ");
			else*/
				fmt::print("Warning: ");
			
			fmt::print(fmt, std::forward<Args>(args)...);
			std::fputc('\n', stdout);
		}
	}

	template<bool active, typename... Args>
	constexpr void print_error(fmt::format_string<Args...> fmt, Args && ...args)
	{
		if constexpr (active)
		{
			/*if (args::GlobalArguments->colored_output)
				fmt::print(fg(fmt::color::red) | fmt::emphasis::bold, "Error: ");
			else*/
				fmt::print("Error: ");
			
			fmt::print(fmt, std::forward<Args>(args)...);
			std::fputc('\n', stdout);
		}
	}
}

template<>
/// @brief {fmt} specialization of EscapeChar
struct fmt::formatter<colt::io::EscapeChar>
{
	template<typename ParseContext>
	constexpr auto parse(ParseContext& ctx) { return ctx.begin(); }

	template<typename FormatContext>
	auto format(const colt::io::EscapeChar& chr, FormatContext& ctx)
	{
		const char* ret;
		switch (chr.chr)
		{
		break; case '\'':
			ret = "\\'";
		break; case '\"':
			ret = "\\\"";
		break; case '\\':
			ret = "\\";
		break; case '\0':
			ret = "\\0";
		break; case '\a':
			ret = "\\a";
		break; case '\b':
			ret = "\\b";
		break; case '\f':
			ret = "\\f";
		break; case '\n':
			ret = "\\n";
		break; case '\r':
			ret = "\\r";
		break; case '\t':
			ret = "\\t";
		break; case '\v':
			ret = "\\v";
		break; default:
			ret = nullptr;
		}
		if (ret == nullptr)
			return fmt::format_to(ctx.out(), "'{}'", chr.chr);
		return fmt::format_to(ctx.out(), "'{}'", ret);
	}
};

#endif //!HG_COLT_PRINT