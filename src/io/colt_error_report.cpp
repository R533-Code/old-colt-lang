#include "colt_error_report.h"
#include "colt_error_report.h"
#include "colt_code_highlight.h"

namespace colt::lang::details
{
	void print_single_line(io::Color highlight, const SourceCodeExprInfo& src_info, StringView begin_line, StringView end_line, size_t line_nb_size) noexcept
	{
		io::Print(" {} | {}{}{}{}{}", src_info.line_begin, io::HighlightCode{ begin_line },
			highlight, src_info.expression, io::Reset, io::HighlightCode{ end_line });

		auto sz = src_info.expression.get_size();
		//So no overflow happens when the expression is empty
		sz += as<size_t>(sz == 0);
		sz -= 1;
		io::Print(" {: <{}} | {: <{}}{:~<{}}^", "", line_nb_size, "", begin_line.get_size(), "", sz);
	}
	
	void print_multiple_lines(io::Color highlight, const SourceCodeExprInfo& src_info, StringView begin_line, StringView end_line, size_t line_nb_size) noexcept
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

			io::Print(" {: >{}} | {}", current_line, line_nb_size,
				io::HighlightCode{ StringView{ begin_line.get_data() + previous_offset, begin_line.get_data() + offset } });
			++current_line;
		}
		io::Print(" {: >{}} | {}{}{}{}", current_line, line_nb_size,
			io::HighlightCode{ StringView{ begin_line.get_data() + previous_offset, begin_line.end() } }, highlight,
			StringView{ src_info.expression.get_data(), src_info.expression.get_data() + offset }, io::Reset);
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

			io::Print(" {: >{}} | {}{}{}", current_line, line_nb_size, highlight,
				StringView{ src_info.expression.get_data() + previous_offset, src_info.expression.get_data() + offset }, io::Reset);
			++current_line;
		}
		io::Print(" {: >{}} | {}{}{}{}", current_line, line_nb_size, highlight,
			StringView{ src_info.expression.get_data() + previous_offset, src_info.expression.end() }, io::Reset,
			io::HighlightCode{ StringView{ end_line.get_data(), end_line.get_data() + offset } });
		++current_line;
		for (;;)
		{
			previous_offset = offset + 1;
			offset = end_line.find('\n', offset + 1);

			if (offset == StringView::npos)
			{
				if (previous_offset < end_line.get_size())
				{
					io::Print(" {: >{}} | {}", current_line, line_nb_size,
						io::HighlightCode{ StringView{ end_line.get_data() + previous_offset, end_line.end() } });
				}
				break;
			}

			io::Print(" {: >{}} | {}", current_line, line_nb_size,
				io::HighlightCode{ StringView{ end_line.get_data() + previous_offset, end_line.get_data() + offset } });
			++current_line;
		}
	}
}