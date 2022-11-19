/** @file colt_lexer.cpp
* Contains definition of functions declared in 'colt_lexer.h'.
*/

#include "colt_lexer.h"

namespace colt::lang
{
	Lexer::Lexer(StringView strv) noexcept
		: to_scan(strv)
	{
		if (strv.is_empty())
			to_scan = "";
		assert_true(strv.get_back() == '\0', "The StringView should be NUL-terminated!");
	}
	
	Lexer::LineInformations Lexer::get_line_info() const noexcept
	{
		return { current_line, as<u32>(offset - line_begin_old), get_line_strv() };
	}

	Token Lexer::get_next_token() noexcept
	{
		//We skip spaces
		while (isSpace(current_char))
		{
			if (current_char == '\n')
				current_line += 1;
			current_char = get_next_char();
		}

		//we store the current offset, which is the beginning of the current lexeme
		lexeme_begin = offset - 1;

		if (isAlpha(current_char) || current_char == '_')
			return handle_identifier();
		else if (isDigit(current_char))
			return handle_digit();

		switch (current_char)
		{
		case '+':
			return handle_plus();
		case '-':
			return handle_minus();
		case '*':
			return handle_star();
		case '/':
			return handle_slash();
		case '.':
			return handle_dot();
		case '<':
			return handle_less();
		case '>':
			return handle_great();
		case '&':
			return handle_and();
		case '|':
			return handle_or();
		case '^':
			return handle_caret();
		case '=':
			return handle_equal();
		case '!':
			return handle_bang();
		case '%':
			return handle_percent();
		case '"':
			return handle_string_literal();
		case '\'':
			return handle_char_literal();
		case ':':
			current_char = get_next_char();
			return TKN_COLON;
		case '~':
			current_char = get_next_char();
			return TKN_TILDE;
		case ',':
			current_char = get_next_char();
			return TKN_COMMA;
		case '{':
			current_char = get_next_char();
			return TKN_LEFT_CURLY;
		case '}':
			current_char = get_next_char();
			return TKN_RIGHT_CURLY;
		case '(':
			current_char = get_next_char();
			return TKN_LEFT_PAREN;
		case ')':
			current_char = get_next_char();
			return TKN_RIGHT_PAREN;
		case '[':
			current_char = get_next_char();
			return TKN_LEFT_SQUARE;
		case ']':
			current_char = get_next_char();
			return TKN_RIGHT_SQUARE;
		case ';':
			current_char = get_next_char();
			return TKN_SEMICOLON;
		case '\0':
		case EOF:
			return TKN_EOF;
		default:
			gen_error(get_current_lexeme(), "Unexpected character!");
			return TKN_EOF;
		}
	}

	void Lexer::set_to_scan(StringView to_scan) noexcept
	{
		this->to_scan = to_scan;
		temp_str.clear();
		offset = 0;
		lexeme_begin = 0;
		line_begin_old = 0;
		line_begin_new = 0;
		current_line = 1;
		current_char = ' ';
	}
	
	StringView Lexer::get_line_strv() const noexcept
	{
		//If the cached result is still valid, return it
		if (current_line == cached_line_nb)
			return cached_line_strv;

		const char* line_begin = to_scan.get_data() +
			(lexeme_begin - as<size_t>(offset == to_scan.get_size()));
		line_begin -= as<size_t>(*line_begin == '\n');
		while (*line_begin != '\n' && line_begin != to_scan.get_data())
			--line_begin;
		line_begin += as<size_t>(*line_begin == '\n');

		const char* line_end = to_scan.get_data() + (lexeme_begin - static_cast<size_t>(offset == to_scan.get_size()));
		while (*line_end != '\n' && *line_end != '\0')
			++line_end;
		//a StringView's end is non-inclusive, so there is no need to change line_end
		//depending on if it is a '\n' or not.

		//Cache result for faster get_line_strv()
		cached_line_strv = { line_begin, line_end };
		cached_line_nb = current_line;

		return cached_line_strv;
	}

	char Lexer::get_next_char() noexcept
	{
		assert_true(!to_scan.is_empty(), "StringView to scan was empty!");
		if (offset < to_scan.get_size())
		{
			char to_ret = to_scan[offset++];
			if (to_ret == '\n')
				line_begin_old = std::exchange(line_begin_new, as<u32>(offset));
			return to_ret;
		}
		return EOF;
	}

	char Lexer::rewind_char(uint64_t offset) noexcept
	{
		assert_true(this->offset > offset, "Invalid offset!");
		//- 1 as the offset points to the NEXT character, not the current one
		return to_scan[(this->offset -= offset) - 1];
	}
	
	char Lexer::peek_next_char(uint64_t offset) const noexcept
	{
		if (this->offset + offset < to_scan.get_size())
			return to_scan[this->offset + offset];
		return EOF;
	}

	Token Lexer::handle_identifier() noexcept
	{
		//Clear the string
		temp_str.clear();
		temp_str += current_char;

		//Save start of the identifier
		const char* ident_start = to_scan.get_data() + offset - 1;

		current_char = get_next_char();
		while (isAlnum(current_char) || current_char == '_')
		{
			temp_str += current_char;
			current_char = get_next_char();
		}

		//Save the parsed identifier
		parsed_identifier = { ident_start, to_scan.get_data() + offset - 1};
		return get_identifier_or_keyword();
	}
	
	Token Lexer::handle_digit() noexcept
	{
		//Allows some optimization for when the AST needs to extract the values from the Lexer
		parsed_value.u64_v = 0;

		//Clear the string
		temp_str.clear();
		temp_str += current_char;

		if (current_char == '0') //Could be 0x, 0b, 0o
		{
			current_char = get_next_char();
			int base = 10;
			switch (current_char)
			{
			break; case 'x': //HEXADECIMAL
				base = 16;
			break; case 'b': //BINARY
				base = 2;
			break; case 'o': //OCTAL	
				base = 8;
			break; default:
				if (isDigit(current_char) || current_char == '.')
				{
					//We recurse now that we have popped the leading 0
					return get_next_token();
				}
				else
					return str_to_integral();
			}

			current_char = parse_alnum();

			if (temp_str.get_size() == 1) //Contains only the '0'
			{
				const char* range_str;
				switch (current_char)
				{
				break; case 'x':
					range_str = "[0-9a-f]";
				break; case 'b':
					range_str = "[0-1]";
				break; case 'o':
					range_str = "[0-7]";
				break; default: //should never happen
					colt_unreachable("Invalid current char!");
				}
				gen_error(get_current_lexeme(), "Integral literals starting with 0{} should be followed by characters in range {}!", current_char, range_str);
				return TKN_ERROR;
			}
			return str_to_u64(base);
		}

		//Parse as many digits as possible
		current_char = parse_digits();

		bool isfloat = false;
		// [0-9]+ followed by a .[0-9] is a float
		if (current_char == '.')
		{
			current_char = get_next_char();
			if (isDigit(current_char))
			{
				isfloat = true;
				temp_str += '.';
				temp_str += current_char;

				//Parse as many digits as possible
				current_char = parse_digits();
			}
			else
			{
				//The dot is not followed by a digit, this is not a float,
				//but rather should be the dot followed by an identifier for a function call
				current_char = rewind_char();
				return str_to_u64(10);
			}
		}
		
		// [0-9]+(.[0-9]+)?e[+-][0-9]+ is a float
		if (char after_e = peek_next_char(0);
			current_char == 'e' && (after_e == '+' || after_e == '-' || isDigit(after_e)))
		{
			isfloat = true;
			temp_str += current_char;
			current_char = get_next_char();
			if (current_char == '+') //skip the + after the exponent
				current_char = get_next_char();

			temp_str += current_char;

			//Parse as many digits as possible
			current_char = parse_digits();
		}

		if (isfloat)
		{
			switch (get_floating_suffix())
			{
			case TKN_FLOAT_L:
				return str_to_float();
			case TKN_DOUBLE_L:
				return str_to_double();
			default:
				colt_unreachable("Floating suffix was invalid!");
			}
		}
		return str_to_integral();
	}
	
	Token Lexer::handle_string_literal() noexcept
	{
		temp_str.clear();

		//Consume the "
		current_char = get_next_char();

		while (current_char != '"' && current_char != '\n' && current_char != EOF)
		{
			if (current_char == '\\')
			{
				if (auto opt = parse_escape_sequence();
					!opt.is_value())
				{
					do
						current_char = get_next_char();
					while (current_char != ';' && current_char != '\'' && current_char != '\n' && current_char != EOF);
					gen_error(get_current_lexeme(), "Invalid escape sequence!");

					return TKN_ERROR;
				}
				else
					current_char = opt.get_value();
			}
			temp_str += current_char;
			current_char = get_next_char();
		}

		if (current_char == '\n' || current_char == EOF)
		{
			gen_error(get_current_lexeme(), "Unterminated string literal!");
			return TKN_ERROR;
		}
		//Consume the "
		current_char = get_next_char();
		return TKN_STRING_L;
	}
	
	Token Lexer::handle_char_literal() noexcept
	{
		//To simplify conversions
		parsed_value.u64_v = 0;

		//Consume the '
		current_char = get_next_char();
		if (current_char == '\'')
		{
			current_char = get_next_char();
			gen_error(get_current_lexeme(), "A char literal should at least contain a character!");
			return TKN_ERROR;
		}
		else if (current_char == '\\')
		{
			if (auto opt = parse_escape_sequence();
				!opt.is_value())
			{
				do
					current_char = get_next_char();
				while (current_char != ';' && current_char != ')' && current_char != '\n' && current_char != EOF);
				gen_error(get_current_lexeme(), "Invalid escape sequence!");

				return TKN_ERROR;
			}
			else
				current_char = opt.get_value();
		}
		//Save the char in parsed_value
		parsed_value.char_v = current_char;
		current_char = get_next_char();

		if (current_char != '\'')
		{
			while (current_char != ';' && current_char != '\n' && current_char != EOF && current_char != '\'')
				current_char = get_next_char();
			gen_error(get_current_lexeme(), "Invalid char literal!");
			return TKN_ERROR;
		}
		//Consume the ''
		current_char = get_next_char();
		return TKN_CHAR_L;
	}
	
	Token Lexer::handle_plus() noexcept
	{
		current_char = get_next_char();
		switch (current_char)
		{
		case '=':
			current_char = get_next_char();
			return TKN_PLUS_EQUAL;
		case '+':
			current_char = get_next_char();
			return TKN_PLUS_PLUS;
		default:
			return TKN_PLUS;
		}
	}
	
	Token Lexer::handle_minus() noexcept
	{
		current_char = get_next_char();
		switch (current_char)
		{
		case '=':
			current_char = get_next_char();
			return TKN_MINUS_EQUAL;
		case '-':
			current_char = get_next_char();
			return TKN_MINUS_MINUS;
		case '>':
			current_char = get_next_char();
			return TKN_MINUS_GREAT;
		default:
			return TKN_MINUS;
		}
	}
	
	Token Lexer::handle_star() noexcept
	{
		current_char = get_next_char();
		if (current_char == '=')
		{
			current_char = get_next_char();
			return TKN_STAR_EQUAL;
		}
		return TKN_STAR;
	}
	
	Token Lexer::handle_slash() noexcept
	{
		current_char = get_next_char();
		switch (current_char)
		{
		case '=':
			current_char = get_next_char();
			return TKN_SLASH_EQUAL;
		case '/': // one line comment
		{
			current_char = get_next_char();
			while (current_char != EOF && current_char != '\n')
				current_char = get_next_char();
			//No need to modify current line if current_char == '\n':
			//get_next_token will do so
			return get_next_token(); //recurse and return the token after the comment
		}
		case '*': // multi-line comment
		{
			//in the case of an unterminated multi-line comment, we want
			//to print the line of the beginning of the multi-line comment.
			u32 line_count = 0;
			current_char = get_next_char();
			while (current_char != EOF)
			{
				if (current_char == '\n')
					line_count++;
				else if (current_char == '*')
				{
					current_char = get_next_char();
					if (current_char == '/')
					{
						//update the line count
						current_line += line_count;

						//consume closing /
						current_char = get_next_char();
						return get_next_token(); //recurse and return the token after the comment
					}
				}
				current_char = get_next_char();
			}
			gen_error(get_current_lexeme(), "Unterminated multi-line comment!");
			return TKN_EOF; //Compilation should fail directly
		}
		default:
			return TKN_SLASH;
		}
	}
	
	Token Lexer::handle_percent() noexcept
	{
		current_char = get_next_char();
		if (current_char == '=')
		{
			current_char = get_next_char();
			return TKN_PERCENT_EQUAL;
		}
		return TKN_PERCENT;
	}
	
	Token Lexer::handle_equal() noexcept
	{
		current_char = get_next_char();
		switch (current_char)
		{
		case '=':
			current_char = get_next_char();
			return TKN_EQUAL_EQUAL;
		case '>':
			current_char = get_next_char();
			return TKN_EQUAL_GREAT;

		default:
			return TKN_EQUAL;
		}
	}
	
	Token Lexer::handle_bang() noexcept
	{
		current_char = get_next_char();
		if (current_char == '=')
		{
			current_char = get_next_char();
			return TKN_BANG_EQUAL;
		}
		return TKN_BANG;
	}
	
	Token Lexer::handle_dot() noexcept
	{
		current_char = get_next_char();
		if (isDigit(current_char))
		{
			//Clear the string
			temp_str.clear();
			temp_str += '.';
			temp_str += current_char;

			current_char = parse_digits();
			
			// [0-9]+(.[0-9]+)?e[+-][0-9]+ is a float
			if (char after_e = peek_next_char();
				current_char == 'e' && (after_e == '+' || after_e == '-' || isDigit(after_e)))
			{
				temp_str += current_char;
				current_char = get_next_char();
				if (current_char == '+') //skip the + after the exponent
					current_char = get_next_char();

				temp_str += current_char;

				//Parse as many digits as possible
				current_char = parse_digits();
			}
			switch (get_floating_suffix())
			{
			case TKN_FLOAT_L:
				return str_to_float();
			case TKN_DOUBLE_L:
				return str_to_double();
			default:
				colt_unreachable("Floating suffix was invalid!");
			}
		}
		return TKN_DOT;
	}
	
	Token Lexer::handle_less() noexcept
	{
		current_char = get_next_char();
		switch (current_char)
		{
		case '=':
			current_char = get_next_char();
			return TKN_LESS_EQUAL;
		case '<':
			current_char = get_next_char();
			if (current_char == '=')
			{
				current_char = get_next_char();
				return TKN_LESS_LESS_EQUAL;
			}
			return TKN_LESS_LESS;
		default:
			return TKN_LESS;
		}
	}
	
	Token Lexer::handle_great() noexcept
	{
		current_char = get_next_char();
		switch (current_char)
		{
		case '=':
			current_char = get_next_char();
			return TKN_GREAT_EQUAL;
		case '>':
			current_char = get_next_char();
			if (current_char == '=')
			{
				current_char = get_next_char();
				return TKN_GREAT_GREAT_EQUAL;
			}
			return TKN_GREAT_GREAT;
		default:
			return TKN_GREAT;
		}
	}
	
	Token Lexer::handle_and() noexcept
	{
		current_char = get_next_char();
		switch (current_char)
		{
		break; case '=':
			current_char = get_next_char();
			return TKN_AND_EQUAL;
		break; case '&':
			current_char = get_next_char();
			return TKN_AND_AND;
		break; default:
			return TKN_AND;
		}
	}
	
	Token Lexer::handle_or() noexcept
	{
		current_char = get_next_char();
		switch (current_char)
		{
		break; case '=':
			current_char = get_next_char();
			return TKN_OR_EQUAL;
		break; case '|':
			current_char = get_next_char();
			return TKN_OR_OR;
		break; default:
			return TKN_OR;
		}
	}
	
	Token Lexer::handle_caret() noexcept
	{
		current_char = get_next_char();
		switch (current_char)
		{
		break; case '=':
			current_char = get_next_char();
			return TKN_CARET_EQUAL;
		break; default:
			return TKN_CARET;
		}
	}

	char Lexer::parse_alnum() noexcept
	{
		char next_char = get_next_char();
		while (isAlnum(next_char))
		{
			temp_str += next_char;
			next_char = get_next_char();
		}
		return next_char;
	}

	char Lexer::parse_digits() noexcept
	{
		char next_char = get_next_char();
		while (isDigit(next_char))
		{
			temp_str += next_char;
			next_char = get_next_char();
		}
		return next_char;
	}

	Token Lexer::str_to_u64(int base) noexcept
	{
		u64 value = 0;
		auto [ptr, err] = std::from_chars(temp_str.begin(), temp_str.end(), value);
		if (ptr == temp_str.end() && err == std::errc{})
		{
			parsed_value.u64_v = value;
			return TKN_U64_L;
		}
		gen_error(get_current_lexeme(), "Invalid integral literal!");
		return TKN_ERROR;
	}

	Token Lexer::str_to_float() noexcept
	{
		float value = 0.0;
		auto [ptr, err] = std::from_chars(temp_str.begin(), temp_str.end(), value);
		if (ptr == temp_str.end() && err == std::errc{})
		{
			parsed_value.float_v = value;
			return TKN_FLOAT_L;
		}
		gen_error(get_current_lexeme(), "Invalid float literal!");
		return TKN_ERROR;
	}

	Token Lexer::str_to_double() noexcept
	{
		double value = 0.0;
		auto [ptr, err] = std::from_chars(temp_str.begin(), temp_str.end(), value);
		if (ptr == temp_str.end() && err == std::errc{})
		{
			parsed_value.double_v = value;
			return TKN_DOUBLE_L;
		}
		gen_error(get_current_lexeme(), "Invalid double literal!");
		return TKN_ERROR;
	}

	Token Lexer::str_to_integral() noexcept
	{
		Token int_type = get_integral_suffix();
		return str_to_u64(10) == TKN_ERROR ? TKN_ERROR : int_type;
	}

	Optional<char> Lexer::parse_escape_sequence() noexcept
	{
		assert_true(current_char == '\\', "An escape sequence starts with a '\\'");
		current_char = get_next_char();
		char ret;
		switch (current_char)
		{
		break; case '\'':
			ret = '\'';
		break; case '\"':
			ret = '\"';
		break; case '\\':
			ret = '\\';
		break; case '0':
			ret = '\0';
		break; case 'a':
			ret = '\a';
		break; case 'b':
			ret = '\b';
		break; case 'f':
			ret = '\f';
		break; case 'n':
			ret = '\n';
		break; case 'r':
			ret = '\r';
		break; case 't':
			ret = '\t';
		break; case 'v':
			ret = '\v';
		break; default:
			//ERROR
			return None;
		}
		return ret;
	}

	Token Lexer::get_identifier_or_keyword() noexcept
	{
		if (temp_str.get_size() == 1)
			return TKN_IDENTIFIER;

		//GlobalTable of keywords
		//We optimize comparisons by comparing the first character
		switch (temp_str[0])
		{
		break; case 'a':
			if (temp_str == "and")
				return TKN_AND_AND;
		break; case 'b':
			if (temp_str == "break")
				return TKN_KEYWORD_BREAK;
			else if (temp_str == "bool")
				return TKN_KEYWORD_BOOL;
		break; case 'c':
			if (temp_str == "char")
				return TKN_KEYWORD_CHAR;
			else if (temp_str == "case")
				return TKN_KEYWORD_CASE;
			else if (temp_str == "mut")
				return TKN_KEYWORD_MUT;
			else if (temp_str == "continue")
				return TKN_KEYWORD_CONTINUE;
		break; case 'd':
			if (temp_str == "double")
				return TKN_KEYWORD_DOUBLE;
			else if (temp_str == "default")
				return TKN_KEYWORD_DEFAULT;
		break; case 'e':
			if (temp_str == "elif")
				return TKN_KEYWORD_ELIF;
			else if (temp_str == "else")
				return TKN_KEYWORD_ELSE;
			else if (temp_str == "extern")
				return TKN_KEYWORD_EXTERN;
		break; case 'f':
			if (temp_str == "for")
				return TKN_KEYWORD_FOR;
			else if (temp_str == "fn")
				return TKN_KEYWORD_FN;
			else if (temp_str == "false")
			{
				parsed_value.bool_v = false;
				return TKN_BOOL_L;
			}
			else if (temp_str == "float")
				return TKN_KEYWORD_FLOAT;
		break; case 'g':
			if (temp_str == "goto")
				return TKN_KEYWORD_GOTO;
		break; case 'i':
			if (temp_str == "if")
				return TKN_KEYWORD_IF;
			else if (temp_str == "i8")
				return TKN_KEYWORD_I8;
			else if (temp_str == "i16")
				return TKN_KEYWORD_I16;
			else if (temp_str == "i32")
				return TKN_KEYWORD_I32;
			else if (temp_str == "i64")
				return TKN_KEYWORD_I64;
		break; case 'l':
			if (temp_str == "lstring")
				return TKN_KEYWORD_LSTRING;
		break; case 'o':
			if (temp_str == "or")
				return TKN_OR_OR;
		break; case 'P':
			if (temp_str == "PTR")
				return TKN_KEYWORD_PTR;
		break; case 'r':
			if (temp_str == "return")
				return TKN_KEYWORD_RETURN;
		break; case 's':
			if (temp_str == "switch")
				return TKN_KEYWORD_SWITCH;
			else if (temp_str == "sizeof")
				return TKN_KEYWORD_SIZEOF;
		break; case 't':
			if (temp_str == "true")
			{
				parsed_value.bool_v = true;
				return TKN_BOOL_L;
			}
			else if (temp_str == "typeof")
				return TKN_KEYWORD_TYPEOF;
		break; case 'u':
			if (temp_str == "u8")
				return TKN_KEYWORD_U8;
			else if (temp_str == "u16")
				return TKN_KEYWORD_U16;
			else if (temp_str == "u32")
				return TKN_KEYWORD_U32;
			else if (temp_str == "u64")
				return TKN_KEYWORD_U64;
		break; case 'w':
			if (temp_str == "while")
				return TKN_KEYWORD_WHILE;
		break; case 'v':
			if (temp_str == "var")
				return TKN_KEYWORD_VAR;
			else if (temp_str == "void")
				return TKN_KEYWORD_VOID;
		break; default:
			return TKN_IDENTIFIER;
		}
		return TKN_IDENTIFIER;
	}

	Token Lexer::get_floating_suffix() noexcept
	{
		switch (std::tolower(current_char))
		{
		break; case 'f':
			current_char = get_next_char();
			return TKN_FLOAT_L;
		break; case 'd':
			current_char = get_next_char();
			return TKN_DOUBLE_L;
		break; default:
			return TKN_DOUBLE_L;
		}
	}

	Token Lexer::get_integral_suffix() noexcept
	{
		//finite automata's
		switch (std::tolower(current_char))
		{
		break; case 'u':
			switch (current_char = get_next_char())
			{
			break; case '8':
				current_char = get_next_char();
				return TKN_U8_L;
			break; case '1':
				current_char = get_next_char();
				if (current_char == '6')
				{
					current_char = get_next_char();
					return TKN_U16_L;
				}
				else
					current_char = rewind_char(2);
			break; case '3':
				current_char = get_next_char();
				if (current_char == '2')
				{
					current_char = get_next_char();
					return TKN_U32_L;
				}
				else
					current_char = rewind_char(2);
			break; case '6':
				current_char = get_next_char();
				if (current_char == '4')
				{
					current_char = get_next_char();
					return TKN_U64_L;
				}
				else
					current_char = rewind_char(2);
			}
		break; case 'i':
			switch (current_char = get_next_char())
			{
			break; case '8':
				current_char = get_next_char();
				return TKN_I8_L;
			break; case '1':
				current_char = get_next_char();
				if (current_char == '6')
				{
					current_char = get_next_char();
					return TKN_I16_L;
				}
				else
					current_char = rewind_char(2);
			break; case '3':
				current_char = get_next_char();
				if (current_char == '2')
				{
					current_char = get_next_char();
					return TKN_I32_L;
				}
				else
					current_char = rewind_char(2);
			break; case '6':
				current_char = get_next_char();
				if (current_char == '4')
				{
					current_char = get_next_char();
					return TKN_I64_L;
				}
				else
					current_char = rewind_char(2);
			}
		}
		return TKN_I64_L;
	}
}