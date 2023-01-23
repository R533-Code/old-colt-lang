/** @file colt_error_report.h
* Contains helpers function for pretty printing messages, warnings and errors.
*/

#ifndef HG_COLT_ERROR_REPORT
#define HG_COLT_ERROR_REPORT

#include <io/console_colors.h>
#include <util/colt_pch.h>

namespace colt::lang
{
	/// @brief The source code information of an expression.
	struct SourceCodeExprInfo
	{
		/// @brief The beginning line number of the expression
		u32 line_begin = {};
		/// @brief The end line number of the expression
		u32 line_end = {};
		/// @brief StringView over all the lines on which the expression spans
		StringView lines;
		/// @brief StringView over the expression (included in lines)
		StringView expression;

		/// @brief Check if the information are valid (not default constructed)
		/// @return True if neither line_begin nor line_end are 0
		bool is_valid() const noexcept { return line_begin != 0 && line_end != 0; }
		/// @brief Check if the information represents a single line
		/// @return True if line_begin == line_end
		bool is_single_line() const noexcept { return line_begin == line_end; }
	};

	template<typename... Args>
	/// @brief Function pointer type of Generate* functions
	/// @tparam ...Args The arguments type to format
	using report_print_t = void(*)(const SourceCodeExprInfo&, fmt::format_string<Args...>, Args&&...) noexcept;

	template<typename... Args>
	/// @brief Generates a message
	/// @tparam ...Args Parameter pack
	/// @param src_info The source information to highlight
	/// @param fmt The error string format
	/// @param ...args The arguments to format
	void GenerateMessage(const SourceCodeExprInfo& src_info, fmt::format_string<Args...> fmt, Args&&... args) noexcept;

	template<typename... Args>
	/// @brief Generates a warning
	/// @tparam ...Args Parameter pack
	/// @param src_info The source information to highlight
	/// @param fmt The error string format
	/// @param ...args The arguments to format
	void GenerateWarning(const SourceCodeExprInfo& src_info, fmt::format_string<Args...> fmt, Args&&... args) noexcept;

	template<typename... Args>
	/// @brief Generates an error
	/// @tparam ...Args Parameter pack
	/// @param src_info The source information to highlight
	/// @param fmt The error string format
	/// @param ...args The arguments to format
	void GenerateError(const SourceCodeExprInfo& src_info, fmt::format_string<Args...> fmt, Args&&... args) noexcept;
	
	namespace details
	{
		void print_single_line(io::Color highlight, const SourceCodeExprInfo& src_info, StringView begin_line, StringView end_line, size_t line_nb_size) noexcept;

		void print_multiple_lines(io::Color highlight, const SourceCodeExprInfo& src_info, StringView begin_line, StringView end_line, size_t line_nb_size) noexcept;		
	}

	template<typename ...Args>
	void GenerateMessage(const SourceCodeExprInfo& src_info, fmt::format_string<Args...> fmt, Args&& ...args) noexcept
	{
		if (args::NoMessage)
			return;
		io::PrintMessage(fmt, std::forward<Args>(args)...);

		if (!src_info.is_valid())
			return;
		
		StringView begin_line = { src_info.lines.get_data(), src_info.expression.get_data() };
		StringView end_line = { src_info.lines.get_data() + src_info.expression.get_size(), src_info.lines.get_data() + src_info.lines.get_size() };

		//If lexeme.get_size() == 0, then the lexeme will be outside of the line_strv:
		//This is because the only case where the lexeme is empty is due to reaching
		//the last lexeme.
		if (src_info.expression.get_size() == 0)
		{
			end_line = StringView{ src_info.expression.get_data(), src_info.expression.get_data() };
			begin_line = src_info.lines;
		}
		else
			end_line = StringView{ src_info.expression.get_data() + src_info.expression.get_size(), src_info.lines.get_data() + src_info.lines.get_size() };
		
		size_t line_nb_size = fmt::formatted_size("{}", src_info.line_end);
		if (src_info.is_single_line())
			details::print_single_line(io::CyanF, src_info, begin_line, end_line, line_nb_size);
		else
			details::print_multiple_lines(io::CyanF, src_info, begin_line, end_line, line_nb_size);
	}

	template<typename ...Args>
	void GenerateWarning(const SourceCodeExprInfo& src_info, fmt::format_string<Args...> fmt, Args&& ...args) noexcept
	{
		if (args::NoWarning)
			return;
		io::PrintWarning(fmt, std::forward<Args>(args)...);

		if (!src_info.is_valid())
			return;

		StringView begin_line = { src_info.lines.get_data(), src_info.expression.get_data() };
		StringView end_line = { src_info.lines.get_data() + src_info.expression.get_size(), src_info.lines.get_data() + src_info.lines.get_size() };

		//If lexeme.get_size() == 0, then the lexeme will be outside of the line_strv:
		//This is because the only case where the lexeme is empty is due to reaching
		//the last lexeme.
		if (src_info.expression.get_size() == 0)
		{
			end_line = StringView{ src_info.expression.get_data(), src_info.expression.get_data() };
			begin_line = src_info.lines;
		}
		else
			end_line = StringView{ src_info.expression.get_data() + src_info.expression.get_size(), src_info.lines.get_data() + src_info.lines.get_size() };
		
		size_t line_nb_size = fmt::formatted_size("{}", src_info.line_end);
		if (src_info.is_single_line())
			details::print_single_line(io::YellowF, src_info, begin_line, end_line, line_nb_size);
		else
			details::print_multiple_lines(io::YellowF, src_info, begin_line, end_line, line_nb_size);
	}
	
	template<typename ...Args>
	void GenerateError(const SourceCodeExprInfo& src_info, fmt::format_string<Args...> fmt, Args&&... args) noexcept
	{
		if (args::NoError)
			return;
		io::PrintError(fmt, std::forward<Args>(args)...);

		if (!src_info.is_valid())
			return;

		StringView begin_line = { src_info.lines.get_data(), src_info.expression.get_data() };
		StringView end_line = { src_info.lines.get_data() + src_info.expression.get_size(), src_info.lines.get_data() + src_info.lines.get_size() };

		//If lexeme.get_size() == 0, then the lexeme will be outside of the line_strv:
		//This is because the only case where the lexeme is empty is due to reaching
		//the last lexeme.
		if (src_info.expression.get_size() == 0)
		{
			end_line = StringView{ src_info.expression.get_data(), src_info.expression.get_data() };
			begin_line = src_info.lines;
		}
		else
			end_line = StringView{ src_info.expression.get_data() + src_info.expression.get_size(), src_info.lines.get_data() + src_info.lines.get_size() };

		size_t line_nb_size = fmt::formatted_size("{}", src_info.line_end);
		if (src_info.is_single_line())
			details::print_single_line(io::BrightRedB, src_info, begin_line, end_line, line_nb_size);
		else
			details::print_multiple_lines(io::BrightRedB, src_info, begin_line, end_line, line_nb_size);
	}
}

#endif //!HG_COLT_ERROR_REPORT