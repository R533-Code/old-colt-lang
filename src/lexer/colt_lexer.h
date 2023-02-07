/** @file colt_lexer.h
* Contains the class responsible of breaking a string into the lexemes of Colt.
*/

#ifndef HG_COLT_SCANNER
#define HG_COLT_SCANNER

#include <util/colt_pch.h>
#include <lexer/colt_token.h>
#include <io/colt_error_report.h>


namespace colt::lang
{
	/// @brief Responsible of breaking a string_view into lexemes
	class Lexer
	{
		/// @brief The string view to scan
		StringView to_scan = {};
		/// @brief The currently parsed lexeme
		StringView parsed_identifier = {};
		/// @brief The last parsed literal value
		QWORD parsed_value = {};

		/// @brief Temporary buffer for parsing
		String temp_str = {};

		/// @brief The offset to the current character from the string
		size_t offset = 0;
		/// @brief The offset to the beginning of the current lexeme
		size_t lexeme_begin = 0;		
		/// @brief The offset to the beginning of the line being parsed
		u32 line_begin_old = 0;
		/// @brief The offset to the beginning of the line being parsed
		u32 line_begin_new = 0;
		/// @brief The current line number
		u32 current_line = 1;

		/// @brief The cached line number
		mutable u32 cached_line_nb = 0;
		/// @brief The cached line StringView
		mutable StringView cached_line_strv = {};
		/// @brief Number of skipped spaces
		u64 skipped_spaces = 0;
		/// @brief The current char, which is the one to parse next
		char current_char = ' ';
		/// @brief If false, then errors are not reported to the console
		bool report_errors = true;

		/// @brief Contains informations about the current line being parsed
		struct LineInformations
		{
			/// @brief The line number
			u32 line_nb = 0;
			/// @brief The column of the current char
			u32 char_column = 0;
			/// @brief String view over the line being parsed
			StringView line_strv = {};
		};

	public:
		/**************** CONSTRUCTORS ****************/

		/// @brief Default construct the Lexer
		Lexer() noexcept = default;
		/// @brief Constructs a Lexer to parse 'strv'
		/// @param strv A NUL terminated StringView
		Lexer(StringView strv, bool report_errors = true) noexcept;
		/// @brief Default destructor
		~Lexer() noexcept = default;
		/// @brief Default move constructor
		Lexer(Lexer&&) noexcept = default;
		/// @brief Default move assignment operator
		Lexer& operator=(Lexer&&) noexcept = default;
		/// @brief Default copy constructor
		Lexer(const Lexer&) noexcept = default;
		/// @brief Default copy assignment operator
		Lexer& operator=(const Lexer&) noexcept = default;

		/**************** GETTERS ****************/

		/// @brief Returns the number of spaces skipped before hitting the lexeme
		/// @return Number of spaces skipped
		u64 get_skipped_spaces_count() const noexcept { return skipped_spaces; }
		
		/// @brief Returns the current offset into the StringView to parse
		/// @return Byte offset from the beginning of the StringView
		u64 get_current_offset() const noexcept { return offset; }

		/// @brief Returns line informations of the current lexeme
		/// @return Line informations of the current lexeme
		LineInformations get_line_info() const noexcept;

		/// @brief Get the last parsed identifier
		/// @return String view over the identifier
		StringView get_parsed_identifier() const noexcept { return parsed_identifier; }
		
		/// @brief Get the last parsed literal value
		/// @return Union of the possible value
		QWORD get_parsed_value() const noexcept { return parsed_value; }

		/// @brief Returns the current line number
		/// @return Current line number being parsed
		u32 get_current_line() const noexcept { return current_line; }

		/// @brief Get the current lexeme
		/// @return String view over the current lexeme
		StringView get_current_lexeme() const noexcept {
			return { to_scan.get_data() + lexeme_begin, to_scan.get_data() + offset - 1 };
		}

		/// @brief Returns the parsed String literal
		/// @return String literal
		String get_string_literal() const noexcept { return temp_str; }

		/// @brief Returns the next token from the string to parse
		/// @return Token representing the parsed lexeme
		Token get_next_token() noexcept;

		/// @brief Resets the state of the scanner and 
		/// @param to_scan The StringView to scan
		void set_to_scan(StringView to_scan, bool report_errors = true) noexcept;

	private:
		/// @brief Returns a string view over the current line
		/// @return String view over the current line
		StringView get_line_strv() const noexcept;

		/// @brief Returns the next char in the string to parse
		/// @return The next character or EOF if no more characters can be found
		char get_next_char() noexcept;

		/// @brief Rewind by 'offset' character
		/// @param offset The offset to rewind
		/// @return The character 'offset' before the current one
		char rewind_char(uint64_t offset = 1) noexcept;

		/// @brief Look ahead in the string to scan
		/// @param offset The offset to add (0 being look ahead 1 character)
		/// @return The character 'offset + 1' after the current one
		char peek_next_char(uint64_t offset = 0) const noexcept;
		
		/// @brief Handles identifiers and keywords 
		Token handle_identifier() noexcept;

		/// @brief Handles floating points, integer (0[xbo])
		Token handle_digit() noexcept;

		/// @brief Handles ".*" 
		Token handle_string_literal() noexcept;
		
		/// @brief Handles '.' 
		Token handle_char_literal() noexcept;

		/// @brief Handles +, +=, ++
		Token handle_plus() noexcept;

		/// @brief Handles -, -=, --
		Token handle_minus() noexcept;
		
		/// @brief Handles *, *=			
		Token handle_star() noexcept;

		/// @brief Handles /, /= and comments
		Token handle_slash() noexcept;

		/// @brief Handles %, &=
		Token handle_percent() noexcept;

		/// @brief Handles = and ==
		Token handle_equal() noexcept;

		/// @brief Handles ! and !=
		Token handle_bang() noexcept;

		/// @brief Handles . which can be a dot or a float 
		Token handle_dot() noexcept;

		/// @brief Handles <, <=, <<, <<=
		Token handle_less() noexcept;

		/// @brief Handles >, >=, >>, >>=
		Token handle_great() noexcept;

		/// @brief Handles &, &=, &&
		Token handle_and() noexcept;

		/// @brief Handles |, |=, ||
		Token handle_or() noexcept;

		/// @brief Handles ^, ^=
		Token handle_caret() noexcept;

		/// @brief Handles '@' 
		Token handle_at() noexcept;

		/// @brief Parses digits greedily, storing in temp_str
		/// @return The first non-digit char
		char parse_digits() noexcept;

		/// @brief Parses alphanumerics greedily, storing in temp_str
		/// @return The first non-alphanumeric char
		char parse_alnum() noexcept;

		/// @brief Converts 'temp_str' to an integer and checks for the suffix
		/// @return Any integral token or TKN_ERROR
		Token str_to_integral() noexcept;

		/// @brief Converts 'temp_str' to an integer
		/// @param base The base of the integer being parsed
		/// @return valid or TKN_ERROR
		template<typename T, Token valid, typename = std::enable_if_t<std::is_integral_v<T>>>
		Token str_to_integral(int base) noexcept;
		
		/// @brief Converts 'temp_str' to a float
		/// @return TKN_FLOAT_L or TKN_ERROR
		Token str_to_float() noexcept;
		
		/// @brief Converts 'temp_str' to a double
		/// @return TKN_DOUBLE_L or TKN_ERROR
		Token str_to_double() noexcept;

		/// @brief Parses an integral suffix ([uUiI]8|16|32|64).
		/// Defaults to i64.
		/// @return Any integral token
		Token get_integral_suffix() noexcept;

		/// @brief Parses a floating point suffix ([fFdD]).
		/// Defaults to a double.
		/// @return Any floating token
		Token get_floating_suffix() noexcept;

		/// @brief Handles the content of 'temp_str' checking if it contains a keyword or is an identifier.
		/// @return Any keyword token or TKN_IDENTIFIER
		Token get_identifier_or_keyword() noexcept;

		/// @brief Parses any escape sequence for a char.
		/// Should be called if a '\' was detected.
		/// @return A valid char or nullopt
		Optional<char> parse_escape_sequence() noexcept;

		/// @brief Consumes all chars till a '\n' or EOF is hit
		/// @return The character that stopped the consume
		char consume_line() noexcept;

		template<typename... Args>
		/// @brief Generates an error
		/// @tparam ...Args The parameter pack to format
		/// @param lexeme The lexeme to highlight
		/// @param fmt The format string
		/// @param ...args The argument pack to format
		void gen_error(StringView lexeme, fmt::format_string<Args...> fmt, Args&&... args) noexcept;

		template<typename... Args>
		/// @brief Generates an error
		/// @tparam ...Args The parameter pack to format
		/// @param lexeme The lexeme to highlight
		/// @param fmt The format string
		/// @param ...args The argument pack to format
		void gen_warn(StringView lexeme, fmt::format_string<Args...> fmt, Args&&... args) noexcept;
	};

	template<typename T, Token valid, typename>
	inline Token Lexer::str_to_integral(int base) noexcept
	{
		T value = 0;
		auto [ptr, err] = std::from_chars(temp_str.begin(), temp_str.end(), value, base);
		if (ptr == temp_str.end() && err == std::errc{})
		{
			parsed_value = value;
			return valid;
		}
		if constexpr (std::is_same_v<T, i8>)
			gen_error(get_current_lexeme(), "Invalid 'i8' integer literal!");
		if constexpr (std::is_same_v<T, u8>)
			gen_error(get_current_lexeme(), "Invalid 'u8' integer literal!");
		if constexpr (std::is_same_v<T, i16>)
			gen_error(get_current_lexeme(), "Invalid 'i16' integer literal!");
		if constexpr (std::is_same_v<T, u16>)
			gen_error(get_current_lexeme(), "Invalid 'u16' integer literal!");
		if constexpr (std::is_same_v<T, i32>)
			gen_error(get_current_lexeme(), "Invalid 'i32' integer literal!");
		if constexpr (std::is_same_v<T, u32>)
			gen_error(get_current_lexeme(), "Invalid 'u32' integer literal!");
		if constexpr (std::is_same_v<T, i64>)
			gen_error(get_current_lexeme(), "Invalid 'i64' integer literal!");
		if constexpr (std::is_same_v<T, u64>)
			gen_error(get_current_lexeme(), "Invalid 'u64' integer literal!");
		return TKN_ERROR;
	}

	template<typename ...Args>
	inline void Lexer::gen_error(StringView lexeme, fmt::format_string<Args...> fmt, Args&&... args) noexcept
	{
		if (!report_errors)
			return;
		auto info = get_line_info();
		//Construct source information from lexeme information
		SourceCodeExprInfo lexeme_info = { info.line_nb, info.line_nb, info.line_strv, get_current_lexeme() };
		
		GenerateError(lexeme_info, fmt, std::forward<Args>(args)...);
	}

	template<typename ...Args>
	inline void Lexer::gen_warn(StringView lexeme, fmt::format_string<Args...> fmt, Args&&... args) noexcept
	{
		if (!report_errors)
			return;
		auto info = get_line_info();
		//Construct source information from lexeme information
		SourceCodeExprInfo lexeme_info = { info.line_nb, info.line_nb, info.line_strv, get_current_lexeme() };

		GenerateWarning(lexeme_info, fmt, std::forward<Args>(args)...);
	}
}

#endif //!HG_COLT_SCANNER