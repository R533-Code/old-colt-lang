/** @file colt_error_report.h
* Contains helpers function for pretty printing messages, warnings and errors.
*/

#ifndef HG_COLT_ERROR_REPORT
#define HG_COLT_ERROR_REPORT

#include <util/console_colors.h>
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
	
	template<typename ...Args>
	void GenerateMessage(const SourceCodeExprInfo& src_info, fmt::format_string<Args...> fmt, Args&& ...args) noexcept
	{
		if (!args::GlobalArguments.print_messages)
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
		{
			if (args::GlobalArguments.colored_output)
				io::Print("{} | {}" CONSOLE_FOREGROUND_CYAN "{}" CONSOLE_COLOR_RESET "{}", src_info.line_begin, begin_line, src_info.expression, end_line);
			else
				io::Print("{} | {}{}{}", src_info.line_begin, begin_line, src_info.expression, end_line);
		
			auto sz = src_info.expression.get_size();
			//So no overflow happens when the expression is empty
			sz += as<size_t>(sz == 0);
			sz -= 1;
			io::Print("{: <{}} | {: <{}}{:~<{}}^", "", line_nb_size, "", begin_line.get_size(), "", sz);
		}
		else
		{
			size_t offset = StringView::npos; //will overflow on first add
			size_t previous_offset = 0;
			size_t current_line = src_info.line_begin;
			for (;;)
			{
				// +1 to skip over '\n'
				//As offset start with npos, offset + 1 == 0 on first iteration
				previous_offset = offset + 1;
				offset = begin_line.find('\n', offset + 1);

				if (offset == StringView::npos)
				{
					offset = src_info.expression.find('\n', 0);
					offset *= as<size_t>(offset != StringView::npos);
					break;
				}

				io::Print("{: >{}} | {}", current_line, line_nb_size,
					StringView{ begin_line.get_data() + previous_offset, begin_line.get_data() + offset });
				++current_line;
			}
			io::Print("{: >{}} | {}" CONSOLE_FOREGROUND_CYAN "{}" CONSOLE_COLOR_RESET, current_line, line_nb_size,
				StringView{ begin_line.get_data() + previous_offset, begin_line.end() },
				StringView{ src_info.expression.get_data(), src_info.expression.get_data() + offset });
			++current_line;
			for (;;)
			{
				previous_offset = offset + 1;
				offset = src_info.expression.find('\n', offset + 1);

				if (offset == StringView::npos)
				{
					offset = end_line.find('\n', 0);
					offset *= as<size_t>(offset != StringView::npos);
					offset += end_line.get_size() * as<size_t>(offset != StringView::npos);
					break;
				}

				io::Print("{: >{}} | " CONSOLE_FOREGROUND_CYAN "{}" CONSOLE_COLOR_RESET, current_line, line_nb_size,
					StringView{ src_info.expression.get_data() + previous_offset, src_info.expression.get_data() + offset });
				++current_line;
			}
			io::Print("{: >{}} | " CONSOLE_FOREGROUND_CYAN "{}" CONSOLE_COLOR_RESET "{}", current_line, line_nb_size,
				StringView{ src_info.expression.get_data() + previous_offset, src_info.expression.end() },
				StringView{ end_line.get_data(), end_line.get_data() + offset });
			++current_line;
			for (;;)
			{
				previous_offset = offset + 1;
				offset = end_line.find('\n', offset + 1);

				if (offset == StringView::npos)
				{
					if (previous_offset < end_line.get_size())
					{
						io::Print("{: >{}} | {}", current_line, line_nb_size,
							StringView{ end_line.get_data() + previous_offset, end_line.end() });
					}
					break;
				}

				io::Print("{: >{}} | {}", current_line, line_nb_size,
					StringView{ end_line.get_data() + previous_offset, end_line.get_data() + offset });
				++current_line;
			}
		}
	}

	template<typename ...Args>
	void GenerateWarning(const SourceCodeExprInfo& src_info, fmt::format_string<Args...> fmt, Args&& ...args) noexcept
	{
		if (!args::GlobalArguments.print_warnings)
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
		{
			if (args::GlobalArguments.colored_output)
				io::Print("{} | {}" CONSOLE_FOREGROUND_YELLOW "{}" CONSOLE_COLOR_RESET "{}", src_info.line_begin, begin_line, src_info.expression, end_line);
			else
				io::Print("{} | {}{}{}", src_info.line_begin, begin_line, src_info.expression, end_line);

			auto sz = src_info.expression.get_size();
			//So no overflow happens when the expression is empty
			sz += as<size_t>(sz == 0);
			sz -= 1;
			io::Print("{: <{}} | {: <{}}{:~<{}}^", "", line_nb_size, "", begin_line.get_size(), "", sz);
		}
		else
		{
			size_t offset = StringView::npos; //will overflow on first add
			size_t previous_offset = 0;
			size_t current_line = src_info.line_begin;
			for (;;)
			{
				// +1 to skip over '\n'
				//As offset start with npos, offset + 1 == 0 on first iteration
				previous_offset = offset + 1;
				offset = begin_line.find('\n', offset + 1);

				if (offset == StringView::npos)
				{
					offset = src_info.expression.find('\n', 0);
					offset *= as<size_t>(offset != StringView::npos);
					break;
				}

				io::Print("{: >{}} | {}", current_line, line_nb_size,
					StringView{ begin_line.get_data() + previous_offset, begin_line.get_data() + offset });
				++current_line;
			}
			io::Print("{: >{}} | {}" CONSOLE_FOREGROUND_YELLOW "{}" CONSOLE_COLOR_RESET, current_line, line_nb_size,
				StringView{ begin_line.get_data() + previous_offset, begin_line.end() },
				StringView{ src_info.expression.get_data(), src_info.expression.get_data() + offset });
			++current_line;
			for (;;)
			{
				previous_offset = offset + 1;
				offset = src_info.expression.find('\n', offset + 1);

				if (offset == StringView::npos)
				{
					offset = end_line.find('\n', 0);
					offset *= as<size_t>(offset != StringView::npos);
					offset += end_line.get_size() * as<size_t>(offset != StringView::npos);
					break;
				}

				io::Print("{: >{}} | " CONSOLE_FOREGROUND_YELLOW "{}" CONSOLE_COLOR_RESET, current_line, line_nb_size,
					StringView{ src_info.expression.get_data() + previous_offset, src_info.expression.get_data() + offset });
				++current_line;
			}
			io::Print("{: >{}} | " CONSOLE_FOREGROUND_YELLOW "{}" CONSOLE_COLOR_RESET "{}", current_line, line_nb_size,
				StringView{ src_info.expression.get_data() + previous_offset, src_info.expression.end() },
				StringView{ end_line.get_data(), end_line.get_data() + offset });
			++current_line;
			for (;;)
			{
				previous_offset = offset + 1;
				offset = end_line.find('\n', offset + 1);

				if (offset == StringView::npos)
				{
					if (previous_offset < end_line.get_size())
					{
						io::Print("{: >{}} | {}", current_line, line_nb_size,
							StringView{ end_line.get_data() + previous_offset, end_line.end() });
					}
					break;
				}

				io::Print("{: >{}} | {}", current_line, line_nb_size,
					StringView{ end_line.get_data() + previous_offset, end_line.get_data() + offset });
				++current_line;
			}
		}
	}
	
	template<typename ...Args>
	void GenerateError(const SourceCodeExprInfo& src_info, fmt::format_string<Args...> fmt, Args&&... args) noexcept
	{
		if (!args::GlobalArguments.print_errors)
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
		{
			if (args::GlobalArguments.colored_output)
				io::Print("{} | {}" CONSOLE_BACKGROUND_BRIGHT_RED "{}" CONSOLE_COLOR_RESET "{}", src_info.line_begin, begin_line, src_info.expression, end_line);
			else
				io::Print("{} | {}{}{}", src_info.line_begin, begin_line, src_info.expression, end_line);

			auto sz = src_info.expression.get_size();
			//So no overflow happens when the expression is empty
			sz += as<size_t>(sz == 0);
			sz -= 1;
			io::Print("{: <{}} | {: <{}}{:~<{}}^", "", line_nb_size, "", begin_line.get_size(), "", sz);
		}
		else
		{
			size_t offset = StringView::npos; //will overflow on first add
			size_t previous_offset = 0;
			size_t current_line = src_info.line_begin;
			for (;;)
			{
				// +1 to skip over '\n'
				//As offset start with npos, offset + 1 == 0 on first iteration
				previous_offset = offset + 1;
				offset = begin_line.find('\n', offset + 1);

				if (offset == StringView::npos)
				{
					offset = src_info.expression.find('\n', 0);
					offset *= as<size_t>(offset != StringView::npos);
					break;
				}

				io::Print("{: >{}} | {}", current_line, line_nb_size,
					StringView{ begin_line.get_data() + previous_offset, begin_line.get_data() + offset });
				++current_line;
			}
			io::Print("{: >{}} | {}" CONSOLE_BACKGROUND_BRIGHT_RED "{}" CONSOLE_COLOR_RESET, current_line, line_nb_size,
				StringView{ begin_line.get_data() + previous_offset, begin_line.end() },
				StringView{ src_info.expression.get_data(), src_info.expression.get_data() + offset });
			++current_line;
			for (;;)
			{
				previous_offset = offset + 1;
				offset = src_info.expression.find('\n', offset + 1);

				if (offset == StringView::npos)
				{
					offset = end_line.find('\n', 0);
					offset *= as<size_t>(offset != StringView::npos);
					offset += end_line.get_size() * as<size_t>(offset != StringView::npos);
					break;
				}

				io::Print("{: >{}} | " CONSOLE_BACKGROUND_BRIGHT_RED "{}" CONSOLE_COLOR_RESET, current_line, line_nb_size,
					StringView{ src_info.expression.get_data() + previous_offset, src_info.expression.get_data() + offset });
				++current_line;
			}
			io::Print("{: >{}} | " CONSOLE_BACKGROUND_BRIGHT_RED "{}" CONSOLE_COLOR_RESET "{}", current_line, line_nb_size,
				StringView{ src_info.expression.get_data() + previous_offset, src_info.expression.end() },
				StringView{ end_line.get_data(), end_line.get_data() + offset });
			++current_line;
			for (;;)
			{
				previous_offset = offset + 1;
				offset = end_line.find('\n', offset + 1);

				if (offset == StringView::npos)
				{
					if (previous_offset < end_line.get_size())
					{
						io::Print("{: >{}} | {}", current_line, line_nb_size,
							StringView{ end_line.get_data() + previous_offset, end_line.end() });
					}
					break;
				}

				io::Print("{: >{}} | {}", current_line, line_nb_size,
					StringView{ end_line.get_data() + previous_offset, end_line.get_data() + offset });
				++current_line;
			}
		}
	}
}

#endif //!HG_COLT_ERROR_REPORT