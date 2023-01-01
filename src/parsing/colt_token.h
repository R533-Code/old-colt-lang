/** @file colt_token.h
* Contains an enum for all valid tokens (representing a lexeme) of the Colt language.
*/

#ifndef HG_COLT_TOKEN
#define HG_COLT_TOKEN

#include "util/colt_macro.h"

namespace colt::lang
{
	/// @brief Represents the lexeme of the Colt language.
	/// To optimize switch and category checks, the Token
	/// is a bit complex in its layout and order.
	/// Comments explains how to add new Tokens without
	/// causing problems with `is*Token` functions.
	enum Token
		: uint8_t
	{
		/********* BEGINNING OF BINARY OPERATORS *******/
		/// While not all symbols in this section are binary
		/// operators, some are considered as such to simplify
		/// Pratt's Parsing in the AST.

		/// @brief +
		TKN_PLUS,
		/// @brief -
		TKN_MINUS,
		/// @brief *
		TKN_STAR,		
		/// @brief /
		TKN_SLASH,		
		/// @brief %
		TKN_PERCENT,
		/// @brief &
		TKN_AND,		
		/// @brief |
		TKN_OR,		
		/// @brief ^
		TKN_CARET,
		/// @brief <<
		TKN_LESS_LESS,
		/// @brief >>
		TKN_GREAT_GREAT,
		
		/// The TKN_GREAT_GREAT is used as a delimiter for comparison
		/// operators: do not add non comparison operators tokens
		/// after it.

		/// @brief &&
		TKN_AND_AND,
		/// @brief ||
		TKN_OR_OR,

		/// @brief <
		TKN_LESS,
		/// @brief <=
		TKN_LESS_EQUAL,

		/// @brief >
		TKN_GREAT,
		/// @brief >=
		TKN_GREAT_EQUAL,
		
		/// @brief !=
		TKN_BANG_EQUAL,
		/// @brief ==
		TKN_EQUAL_EQUAL,

		/// The TKN_EQUAL_EQUAL is used as a delimiter for assignment
		/// operators: do not add non assignment operators tokens
		/// after it.

		/********* BEGINNING OF ASSIGNMENT OPERATORS *******/

		/// The TKN_EQUAL is used as a delimiter for direct assignment
		/// operators: do not add non direct assignment operators tokens
		/// after it.
		/// The TKN_EQUAL is also used as a delimiter for comparison
		/// operators: do not add non comparison operators tokens
		/// before it.

		/// @brief =
		TKN_EQUAL,
		/// @brief +=
		TKN_PLUS_EQUAL,
		/// @brief -=
		TKN_MINUS_EQUAL,
		/// @brief *=
		TKN_STAR_EQUAL,
		/// @brief /=
		TKN_SLASH_EQUAL,
		/// @brief %=
		TKN_PERCENT_EQUAL,
		/// @brief &=
		TKN_AND_EQUAL,
		/// @brief |=
		TKN_OR_EQUAL,
		/// @brief ^=
		TKN_CARET_EQUAL,
		/// @brief <<=
		TKN_LESS_LESS_EQUAL,
		/// @brief >>=
		TKN_GREAT_GREAT_EQUAL,

		/********* END OF ASSIGNMENT OPERATORS *******/

		/// The TKN_COMMA is used as a delimiter for assignment
		/// operators and direct assignment operators: do not add
		/// non (direct) assignment operators tokens before it.

		/// @brief ,
		TKN_COMMA,
		/// @brief ;
		TKN_SEMICOLON,		
		/// @brief end of file
		TKN_EOF,
		/// @brief error detected lexeme
		TKN_ERROR,
		/// @brief )
		TKN_RIGHT_PAREN,
		/// @brief (
		TKN_LEFT_PAREN,


		/// @brief :
		TKN_COLON,
		/// @brief }
		TKN_RIGHT_CURLY,
		/// @brief {
		TKN_LEFT_CURLY,

		/********* END OF BINARY OPERATORS *******/

		/// @brief ->
		TKN_MINUS_GREAT,
		/// @brief =>
		TKN_EQUAL_GREAT,

		/// @brief ++
		TKN_PLUS_PLUS,
		/// @brief --
		TKN_MINUS_MINUS,
		/// @brief ~
		TKN_TILDE,
		/// @brief !
		TKN_BANG,
				
		/// @brief [
		TKN_LEFT_SQUARE,
		/// @brief ]
		TKN_RIGHT_SQUARE,		
		
		/********* BEGINNING OF LITERAL TOKENS *******/

		/// @brief true/false
		TKN_BOOL_L,
		/// @brief '.'
		TKN_CHAR_L,
		/// @brief NUMi8
		TKN_I8_L,
		/// @brief NUMu8
		TKN_U8_L,
		/// @brief NUMi16
		TKN_I16_L,
		/// @brief NUMu16
		TKN_U16_L,
		/// @brief NUMi32
		TKN_I32_L,
		/// @brief NUMu32
		TKN_U32_L,
		/// @brief NUMi64
		TKN_I64_L,
		/// @brief NUMu64
		TKN_U64_L,
		/// @brief REALf
		TKN_FLOAT_L,
		/// @brief REALd
		TKN_DOUBLE_L,
		/// @brief "..."
		TKN_STRING_L,
		
		/********* END OF LITERAL TOKENS *******/
		/********* DO NOT ADD KEYWORDS HERE *******/
		/// The TKN_KEYWORD_EXTERN is used as a delimiter
		/// for literal tokens: do not add non-literal tokens
		/// before it.

		/// @brief extern
		TKN_KEYWORD_EXTERN,
		/// @brief if
		TKN_KEYWORD_IF,
		/// @brief elif
		TKN_KEYWORD_ELIF,
		/// @brief else
		TKN_KEYWORD_ELSE,
		/// @brief fn
		TKN_KEYWORD_FN,
		/// @brief return
		TKN_KEYWORD_RETURN,
		
		/// The TKN_KEYWORD_VAR is used as a delimiter for built-in
		/// types tokens: do not add non-built-in types tokens
		/// after it.

		/// @brief var
		TKN_KEYWORD_VAR,

		/********* BEGINNING OF BUILTIN TYPES *******/

		/// @brief void
		TKN_KEYWORD_VOID,
		/// @brief bool
		TKN_KEYWORD_BOOL,
		/// @brief char
		TKN_KEYWORD_CHAR,
		/// @brief i8
		TKN_KEYWORD_I8,
		/// @brief u8
		TKN_KEYWORD_U8,
		/// @brief i16
		TKN_KEYWORD_I16,
		/// @brief u16
		TKN_KEYWORD_U16,
		/// @brief i32
		TKN_KEYWORD_I32,
		/// @brief u32
		TKN_KEYWORD_U32,
		/// @brief i64
		TKN_KEYWORD_I64,
		/// @brief u64
		TKN_KEYWORD_U64,
		/// @brief float
		TKN_KEYWORD_FLOAT,
		/// @brief double
		TKN_KEYWORD_DOUBLE,
		/// @brief lstring
		TKN_KEYWORD_LSTRING,
		/// @brief mut
		TKN_KEYWORD_MUT,
		/// @brief PTR
		TKN_KEYWORD_PTR,

		/********* END OF BUILTIN TYPES *******/

		/// The TKN_KEYWORD_FOR is used as a delimiter for built-in
		/// types tokens: do not add non-built-in types tokens
		/// before it.

		/// @brief for
		TKN_KEYWORD_FOR,
		/// @brief while
		TKN_KEYWORD_WHILE,
		/// @brief break
		TKN_KEYWORD_BREAK,
		/// @brief continue
		TKN_KEYWORD_CONTINUE,

		/// @brief compile_t
		TKN_KEYWORD_COMPILE_T,
		/// @brief typeof
		TKN_KEYWORD_TYPEOF,
		
		/// @brief sizeof
		TKN_KEYWORD_SIZEOF,
		/// @brief alignof
		TKN_KEYWORD_ALIGNOF,
		/// @brief alignas
		TKN_KEYWORD_ALIGNAS,
		
		/// @brief as
		TKN_KEYWORD_AS,
		/// @brief reinterpret_as
		TKN_KEYWORD_REINTERPRET_AS,
		
		/// @brief switch
		TKN_KEYWORD_SWITCH,
		/// @brief case
		TKN_KEYWORD_CASE,
		/// @brief default
		TKN_KEYWORD_DEFAULT,		

		/// @brief goto
		TKN_KEYWORD_GOTO,

		/********* ADD NEW KEYWORDS BEGINNING HERE *******/


		/// @brief any identifier
		TKN_IDENTIFIER,
		/// @brief \.
		TKN_DOT		
	};

	/// @brief Check if a Token represents any assignment Token (=, +=, ...)
	/// @param tkn The token to check for
	/// @return True if the Token is an assignment Token
	constexpr bool isAssignmentToken(Token tkn) noexcept
	{
		return TKN_EQUAL_EQUAL < tkn
			&& tkn < TKN_COMMA;
	}

	/// @brief Check if Token represents any direct assignment (+=, -=, ...)
	/// @param tkn The token to check for
	/// @return True if the Token is an direct assignment Token
	constexpr bool isDirectAssignmentToken(Token tkn) noexcept
	{
		return TKN_EQUAL < tkn
			&& tkn < TKN_COMMA;
	}

	/// @brief Converts a direct assignment to its non-assigning equivalent.
	/// Example: '+=' -> '+'
	/// @param tkn The direct assignment Token
	/// @return Non-assigning Token
	/// @pre isDirectAssignmentToken(tkn)
	constexpr Token DirectAssignToNonAssignToken(Token tkn) noexcept
	{
		assert_true(isDirectAssignmentToken(tkn), "Expected a Direct Assignment token!");
		return static_cast<Token>(tkn - 19);
	}

	/// @brief Check if a Token represents any comparison Token (==, !=, ...)
	/// '||' and '&&' are considered comparison tokens.
	/// @param tkn The token to check for
	/// @return True if the Token is a comparison Token
	constexpr bool isComparisonToken(Token tkn) noexcept
	{
		return TKN_GREAT_GREAT < tkn
			&& tkn < TKN_EQUAL;
	}

	/// @brief Check if a Token represents any literal token ('.', "...", ...)
	/// @param tkn The token to check for
	/// @return True if the Token is a literal token
	constexpr bool isLiteralToken(Token tkn) noexcept
	{
		return TKN_RIGHT_SQUARE < tkn
			&& tkn < TKN_KEYWORD_EXTERN;
	}
}

#endif //!HG_COLT_TOKEN