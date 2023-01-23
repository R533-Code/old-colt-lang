/** @file colt_print.h
* Contains utilities for printing to the console.
* These utilities might seem redundant looking at colt_log.h.
* The difference is that logging utilities can be completely removed from a build.
*/

#ifndef HG_COLT_PRINT
#define HG_COLT_PRINT

#include <util/colt_config.h>
#include <fmt/core.h>
#include <cmd/colt_args.h>
#include <io/console_colors.h>

namespace colt
{
	u64 rand(u64 a, u64 b) noexcept;
}

namespace colt::args
{
	extern bool NoColor;
}

/// @brief Contains utilities for printing to console
namespace colt::io
{
	/// @brief Prints 'Press any key to continue...' and waits for any key input.
	void PressToContinue() noexcept;

	template<bool new_line = true, typename... Args>
	/// @brief Prints to the standard output
	/// @tparam ...Args Pack of types to format
	/// @param fmt The format string, using {fmt} syntax
	/// @param ...args The arguments to format
	constexpr void Print(fmt::format_string<Args...> fmt, Args&&... args);

	template<bool new_line = true, typename... Args>
	/// @brief Prints a message to 'stdout'
	/// @tparam ...Args Pack of types to format
	/// @param fmt The format string, using {fmt} syntax
	/// @param ...args The arguments to format
	constexpr void PrintMessage(fmt::format_string<Args...> fmt, Args&&... args);

	template<bool new_line = true, typename... Args>
	/// @brief Prints a warning message to 'stdout'
	/// @tparam ...Args Pack of types to format
	/// @param fmt The format string, using {fmt} syntax
	/// @param ...args The arguments to format
	constexpr void PrintWarning(fmt::format_string<Args...> fmt, Args&&... args);

	template<bool new_line = true, typename... Args>
	/// @brief Prints an error message to 'stdout'
	/// @tparam ...Args Pack of types to format
	/// @param fmt The format string, using {fmt} syntax
	/// @param ...args The arguments to format
	constexpr void PrintError(fmt::format_string<Args...> fmt, Args&&... args);

	template<bool new_line = true, typename... Args>
	/// @brief Prints a fatal error message to 'stdout'
	/// @tparam ...Args Pack of types to format
	/// @param fmt The format string, using {fmt} syntax
	/// @param ...args The arguments to format
	constexpr void PrintFatal(fmt::format_string<Args...> fmt, Args && ...args);

	/// @brief Used to escape a character being printed.
	/// Example: Print("{}", EscapeChar{'\\n'}) -> '\\n'
	struct EscapeChar
	{
		/// @brief The char to escape
		char chr;
	};

	/// @brief Used to print each character of a string in a color.
	/// Example: Print("{}", ColorEachStrChar{ "COLT" })
	struct ColorEachStrChar
	{
		/// @brief The string to color
		const char* str;
	};

	template<bool new_line, typename... Args>
	constexpr void Print(fmt::format_string<Args...> fmt, Args && ...args)
	{
		fmt::print(fmt, std::forward<Args>(args)...);
		if constexpr (new_line)
			std::fputc('\n', stdout);
	}

	template<bool new_line, typename... Args>
	constexpr void PrintMessage(fmt::format_string<Args...> fmt, Args && ...args)
	{
		if (args::NoColor)
			fmt::print(fg(fmt::color::cornflower_blue) | fmt::emphasis::bold, "Message: ");
		else
			fmt::print("Message: ");

		fmt::print(fmt, std::forward<Args>(args)...);
		if constexpr (new_line)
			std::fputc('\n', stdout);
	}

	template<bool new_line, typename... Args>
	constexpr void PrintWarning(fmt::format_string<Args...> fmt, Args && ...args)
	{
		if (args::NoColor)
			fmt::print(fg(fmt::color::yellow) | fmt::emphasis::bold, "Warning: ");
		else
			fmt::print("Warning: ");
			
		fmt::print(fmt, std::forward<Args>(args)...);
		if constexpr (new_line)
			std::fputc('\n', stdout);
	}

	template<bool new_line, typename... Args>
	constexpr void PrintError(fmt::format_string<Args...> fmt, Args && ...args)
	{
		if (args::NoColor)
			fmt::print(fg(fmt::color::red) | fmt::emphasis::bold, "Error: ");
		else
			fmt::print("Error: ");
			
		fmt::print(fmt, std::forward<Args>(args)...);
		if constexpr (new_line)
			std::fputc('\n', stdout);
	}

	template<bool new_line, typename... Args>
	constexpr void PrintFatal(fmt::format_string<Args...> fmt, Args && ...args)
	{
		fmt::print("{}Fatal:{}{} ", io::BrightRedB, io::Reset, io::BrightRedF);
		fmt::print(fmt, std::forward<Args>(args)...);
		fmt::print("{}", io::Reset);

		if constexpr (new_line)
			std::fputc('\n', stdout);
	}
}

template<>
/// @brief {fmt} specialization of EscapeChar
struct fmt::formatter<colt::io::EscapeChar>
{
	template<typename ParseContext>
	constexpr auto parse(ParseContext& ctx) { return ctx.begin(); }

	template<typename FormatContext>
	/// @brief fmt overload
	/// @tparam FormatContext The context to write 
	/// @param chr The char to write
	/// @param ctx The context
	/// @return context
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

template<>
/// @brief {fmt} specialization of EscapeChar
struct fmt::formatter<colt::io::ColorEachStrChar>
{
	template<typename ParseContext>
	constexpr auto parse(ParseContext& ctx) { return ctx.begin(); }

	template<typename FormatContext>
	/// @brief fmt overload
	/// @tparam FormatContext The context to write 
	/// @param chr The char to write
	/// @param ctx The context
	/// @return context
	auto format(const colt::io::ColorEachStrChar& chr, FormatContext& ctx)
	{
		if (!colt::args::GlobalArguments.colored_output)
			return fmt::format_to(ctx.out(), "{}", chr.str);

		colt::io::ColorEachStrChar cpy = chr;
		auto iter = ctx.out();
		while (*cpy.str != '\0')
		{
			iter = fmt::format_to(iter, "{}{}", CONSOLE_COLORS[colt::rand(2, 16)], *cpy.str);
			++cpy.str;
		}
		iter = fmt::format_to(iter, "\x1B[0m");
		return iter;
	}
};

#endif //!HG_COLT_PRINT