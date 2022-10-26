/** @file colt_token.h
* Contains an enum for all valid tokens (representing a lexeme) of the Colt language.
*/

#ifndef HG_COLT_TOKEN
#define HG_COLT_TOKEN

namespace colt::lang
{
	/// @brief Represents the lexeme of the Colt language
	enum Token
		: uint8_t
	{		
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
		
		/// @brief var
		TKN_KEYWORD_VAR,
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
		/// @brief ptr
		TKN_KEYWORD_PTR,

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
		
		/// @brief cast
		TKN_KEYWORD_CAST,
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

		/// @brief any identifier
		TKN_IDENTIFIER,
		/// @brief \.
		TKN_DOT		
	};
}

#endif //!HG_COLT_TOKEN