/** @file colt_char.h
* Contains helpers function pretty printing messages, warnings and errors.
*/

#ifndef HG_COLT_ERROR_REPORT
#define HG_COLT_ERROR_REPORT

#include <util/console_colors.h>
#include <util/colt_pch.h>

namespace colt::lang
{
	template<typename... Args>
	void generate_message(size_t line_nb, std::string_view line_strv, std::string_view lexeme, fmt::format_string<Args...> fmt, Args&&... args) noexcept;

	template<typename... Args>
	void generate_warning(size_t line_nb, std::string_view line_strv, std::string_view lexeme, fmt::format_string<Args...> fmt, Args&&... args) noexcept;

	template<typename... Args>
	void generate_error(size_t line_nb, std::string_view line_strv, std::string_view lexeme, fmt::format_string<Args...> fmt, Args&&... args) noexcept;
	
	template<typename ...Args>
	void generate_message(size_t line_nb, std::string_view line_strv, std::string_view lexeme, fmt::format_string<Args...> fmt, Args&& ...args) noexcept
	{
		assert_true(line_strv.find(lexeme) != std::string_view::npos, "lexeme should be part of line!");
		if (!args::GlobalArguments->print_messages)
			return;
		io::print_message(fmt, std::forward<Args>(args)...);
		
		std::string_view begin_line = { line_strv.data(), lexeme.data() };
		std::string_view end_line = { lexeme.data() + lexeme.size(), line_strv.data() + line_strv.size() };
		
		size_t line_nb_size = fmt::formatted_size("{}", line_nb);
		if (args::GlobalArguments->colored_output)
			io::print("{} | {}" CONSOLE_FOREGROUND_CYAN "{}" CONSOLE_COLOR_RESET "{}", line_nb, begin_line, lexeme, end_line);
		else
			io::print("{} | {}{}{}", line_nb, begin_line, lexeme, end_line);
		
		if (lexeme.empty() || lexeme.size() == 1)
			io::print(" | {: <{}}^", "", begin_line.size());
		else
			io::print("{: <{}} | {: <{}}{:~<{}}", "", line_nb_size, "", begin_line.size(), "", lexeme.size());
	}

	template<typename ...Args>
	void generate_warning(size_t line_nb, std::string_view line_strv, std::string_view lexeme, fmt::format_string<Args...> fmt, Args&& ...args) noexcept
	{
		assert_true(line_strv.find(lexeme) != std::string_view::npos, "lexeme should be part of line!");
		if (!args::GlobalArguments->print_warnings)
			return;
		io::print_warning(fmt, std::forward<Args>(args)...);

		std::string_view begin_line = { line_strv.data(), lexeme.data() };
		std::string_view end_line = { lexeme.data() + lexeme.size(), line_strv.data() + line_strv.size() };
		
		size_t line_nb_size = fmt::formatted_size("{}", line_nb);
		if (args::GlobalArguments->colored_output)
			io::print("{} | {}" CONSOLE_FOREGROUND_YELLOW "{}" CONSOLE_COLOR_RESET "{}", line_nb, begin_line, lexeme, end_line);
		else
			io::print("{} | {}{}{}", line_nb, begin_line, lexeme, end_line);
		
		if (lexeme.empty() || lexeme.size() == 1)
			io::print("{: <{}} | {: <{}}^", "", line_nb_size, "", begin_line.size());
		else
			io::print("{: <{}} | {: <{}}{:~<{}}", "", line_nb_size, "", begin_line.size(), "", lexeme.size());
	}
	template<typename ...Args>
	void generate_error(size_t line_nb, std::string_view line_strv, std::string_view lexeme, fmt::format_string<Args...> fmt, Args&& ...args) noexcept
	{
		assert_true(line_strv.find(lexeme) != std::string_view::npos, "lexeme should be part of line!");
		if (!args::GlobalArguments->print_errors)
			return;
		io::print_error(fmt, std::forward<Args>(args)...);

		std::string_view begin_line = { line_strv.data(), lexeme.data() };
		std::string_view end_line = { lexeme.data() + lexeme.size(), line_strv.data() + line_strv.size() };

		size_t line_nb_size = fmt::formatted_size("{}", line_nb);
		if (args::GlobalArguments->colored_output)
			io::print("{} | {}" CONSOLE_BACKGROUND_BRIGHT_RED "{}" CONSOLE_COLOR_RESET "{}", line_nb, begin_line, lexeme, end_line);
		else
			io::print("{} | {}{}{}", line_nb, begin_line, lexeme, end_line);
		
		if (lexeme.empty() || lexeme.size() == 1)
			io::print("{: <{}} | {: <{}}^", "", line_nb_size, "", begin_line.size());
		else
			io::print("{: <{}} | {: <{}}{:~<{}}", "", line_nb_size, "", begin_line.size(), "", lexeme.size());
	}
}

#endif //!HG_COLT_ERROR_REPORT