/** @file colt_operators.h
* Contains enum for all valid operators of the Colt language.
*/

#ifndef HG_COLT_OPERATORS
#define HG_COLT_OPERATORS

#include <util/colt_macro.h>
#include <parsing/colt_token.h>

namespace colt::lang
{
	/// @brief Possible unary operator
	enum class UnaryOperator
		: u8
	{
		/// @brief ++(Expr)
		OP_INCREMENT,
		/// @brief (Expr)++
		OP_DECREMENT,
		/// @brief -(Expr)
		OP_NEGATE,
		/// @brief *(Expr)
		OP_DEREFERENCE,
		/// @brief &(Expr)
		OP_ADDRESSOF,
		/// @brief !(Expr)
		OP_BOOL_NOT,
		/// @brief ~(Expr)
		OP_BIT_NOT
	};

	/// @brief Converts a Token to a UnaryOperator
	/// @param tkn The Token to convert
	/// @return Assertion failure or a valid UnaryOperator
	UnaryOperator TokenToUnaryOperator(Token tkn) noexcept;

	/// @brief Possible binary operators
	enum class BinaryOperator
		: u8
	{
		/*********** ARITHMETIC ***********/

		/// @brief +
		OP_SUM,
		/// @brief -
		OP_SUB,
		/// @brief *
		OP_MUL,
		/// @brief /
		OP_DIV,
		/// @brief %
		OP_MOD,

		/*********** BITWISE ***********/

		/// @brief &
		OP_BIT_AND,
		/// @brief |
		OP_BIT_OR,
		/// @brief ^
		OP_BIT_XOR,
		/// @brief <<
		OP_BIT_LSHIFT,
		/// @brief >>
		OP_BIT_RSHIFT,
		
		/*********** BOOLEANS ***********/

		/// @brief &&
		OP_BOOL_AND,
		/// @brief ||
		OP_BOOL_OR,
		/// @brief <
		OP_LESS,
		/// @brief <=
		OP_LESS_EQUAL,
		/// @brief >
		OP_GREAT,
		/// @brief >=
		OP_GREAT_EQUAL,
		/// @brief !=
		OP_NOT_EQUAL,
		/// @brief ==
		OP_EQUAL,

		/*********** ASSIGNMENTS ***********/

		/// @brief =
		OP_ASSIGN,
		/// @brief +=
		OP_ASSIGN_SUM,
		/// @brief -=
		OP_ASSIGN_SUB,
		/// @brief *=
		OP_ASSIGN_MUL,
		/// @brief /=
		OP_ASSIGN_DIV,
		/// @brief %=
		OP_ASSIGN_MOD,
		/// @brief &=
		OP_ASSIGN_BIT_AND,
		/// @brief |=
		OP_ASSIGN_BIT_OR,
		/// @brief ^=
		OP_ASSIGN_BIT_XOR,
		/// @brief <<=
		OP_ASSIGN_LSHIFT,
		/// @brief >>=
		OP_ASSIGN_RSHIFT,
	};

	/// @brief Converts a Token to a BinaryOperator
	/// @param tkn The Token to convert
	/// @return Assertion failure or a valid BinaryOperator
	BinaryOperator TokenToBinaryOperator(Token tkn) noexcept;
	
	/// @brief Converts a binary operator to an 'lstring'
	/// @param tkn The operator
	/// @return 'lstring' representing the operator
	const char* BinaryOperatorToString(BinaryOperator tkn) noexcept;
}

#endif //!HG_COLT_OPERATORS