/** @file colt_lexer.h
* Contains the class responsible of breaking a string into the lexemes of Colt.
*/

#ifndef HG_COLT_SCANNER
#define HG_COLT_SCANNER

#include <util/colt_pch.h>
#include <parsing/colt_token.h>
#include <parsing/colt_error_report.h>


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
		size_t line_begin_old = 0;
		/// @brief The offset to the beginning of the line being parsed
		size_t line_begin_new = 0;
		/// @brief The current line number
		size_t current_line = 1;
		
		/// @brief The current char, which is the one to parse next
		char current_char = ' ';

		/// @brief Contains informations about the current line being parsed
		struct LineInformations
		{
			/// @brief The line number
			size_t line_nb = 0;
			/// @brief The column of the current char
			size_t char_column = 0;
			/// @brief String view over the line being parsed
			StringView line_strv = {};
		};

	public:
		/**************** CONSTRUCTORS ****************/

		/// @brief Default construct the Lexer
		Lexer() noexcept = default;
		/// @brief Constructs a Lexer to parse 'strv'
		/// @param strv A NUL terminated StringView
		Lexer(StringView strv) noexcept
			: to_scan(strv)
		{
			if (strv.is_empty())
				to_scan = "";
			assert_true(strv.get_back() == '\0', "The StringView should be NUL-terminated!");
		}
		~Lexer() noexcept = default;
		Lexer(Lexer&&) noexcept = default;
		Lexer& operator=(Lexer&&) noexcept = default;
		Lexer(const Lexer&) noexcept = default;
		Lexer& operator=(const Lexer&) noexcept = default;

		/**************** GETTERS ****************/

		/// @brief Returns line informations of the current lexeme
		/// @return Line informations of the current lexeme
		LineInformations get_line_info() const noexcept;

		/// @brief Get the last parsed identifier
		/// @return String view over the identifier
		StringView get_parsed_identifier() const noexcept { return parsed_identifier; }
		
		/// @brief Get the last parsed literal value
		/// @return Union of the possible value
		QWORD get_parsed_value() const noexcept { return parsed_value; }

		/// @brief Get the current lexeme
		/// @return String view over the current lexeme
		StringView get_current_lexeme() const noexcept { return { to_scan.get_data() + lexeme_begin, to_scan.get_data() + offset - 1 }; }

		/// @brief Returns the next token from the string to parse
		/// @return Token representing the parsed lexeme
		Token get_next_token() noexcept;

		/// @brief Resets the state of the scanner and 
		/// @param to_scan The StringView to scan
		void set_to_scan(StringView to_scan) noexcept;

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
		char peek_next_char(uint64_t offset = 0) noexcept;
		
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
		/// @return TKN_U64_L or TKN_ERROR
		Token str_to_u64(int base) noexcept;
		
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

		template<typename... Args>
		/// @brief Generates an error
		/// @tparam ...Args The parameter pack to format
		/// @param lexeme The lexeme to highlight
		/// @param fmt The format string
		/// @param ...args The argument pack to format
		void gen_error(StringView lexeme, fmt::format_string<Args...> fmt, Args&&... args) noexcept;
	};

	template<typename ...Args>
	inline void Lexer::gen_error(StringView lexeme, fmt::format_string<Args...> fmt, Args&&... args) noexcept
	{
		auto info = get_line_info();
		GenerateError(info.line_nb, info.line_strv, lexeme, fmt, std::forward<Args>(args)...);
	}
}

#endif //!HG_COLT_SCANNER