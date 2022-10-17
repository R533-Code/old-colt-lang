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
	{
		/// @brief ++(Expr)
		OP_PRE_INCREMENT,
		/// @brief (Expr)++
		OP_POST_INCREMENT,
		/// @brief --(Expr)
		OP_PRE_DECREMENT,
		/// @brief (Expr)--
		OP_POST_DECREMENT,
		/// @brief +(Expr)
		OP_PLUS,
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
	/// @param is_post Specify if the Token is a pre/post increment/decrement
	/// @return Assertion failure or a valid UnaryOperator
	UnaryOperator TokenToUnaryOperator(Token tkn, bool is_post) noexcept;

	/// @brief Possible binary operators
	enum class BinaryOperator
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
}

template<>
/// @brief {fmt} specialization of BinaryOperator
struct fmt::formatter<colt::lang::BinaryOperator>
{
	template<typename ParseContext>
	constexpr auto parse(ParseContext& ctx) { return ctx.begin(); }

	template<typename FormatContext>
	auto format(const colt::lang::BinaryOperator& op, FormatContext& ctx)
	{
		using namespace colt::lang;
		const char* str;
		switch (op)
		{
			case BinaryOperator::OP_SUM: str = "OP_SUM"; break;
			case BinaryOperator::OP_SUB: str = "OP_SUB"; break;
			case BinaryOperator::OP_MUL: str = "OP_MUL"; break;
			case BinaryOperator::OP_DIV: str = "OP_DIV"; break;
			case BinaryOperator::OP_MOD: str = "OP_MOD"; break;
			case BinaryOperator::OP_BIT_AND: str = "OP_BIT_AND"; break;
			case BinaryOperator::OP_BIT_OR: str = "OP_BIT_OR"; break;
			case BinaryOperator::OP_BIT_XOR: str = "OP_BIT_XOR"; break;
			case BinaryOperator::OP_BIT_LSHIFT: str = "OP_BIT_LSHIFT"; break;
			case BinaryOperator::OP_BIT_RSHIFT: str = "OP_BIT_RSHIFT"; break;
			case BinaryOperator::OP_BOOL_AND: str = "OP_BOOL_AND"; break;
			case BinaryOperator::OP_BOOL_OR: str = "OP_BOOL_OR"; break;
			case BinaryOperator::OP_LESS: str = "OP_LESS"; break;
			case BinaryOperator::OP_LESS_EQUAL: str = "OP_LESS_EQUAL"; break;
			case BinaryOperator::OP_GREAT: str = "OP_GREAT"; break;
			case BinaryOperator::OP_GREAT_EQUAL: str = "OP_GREAT_EQUAL"; break;
			case BinaryOperator::OP_NOT_EQUAL: str = "OP_NOT_EQUAL"; break;
			case BinaryOperator::OP_EQUAL: str = "OP_EQUAL"; break;
			case BinaryOperator::OP_ASSIGN: str = "OP_ASSIGN"; break;
			case BinaryOperator::OP_ASSIGN_SUM: str = "OP_ASSIGN_SUM"; break;
			case BinaryOperator::OP_ASSIGN_SUB: str = "OP_ASSIGN_SUB"; break;
			case BinaryOperator::OP_ASSIGN_MUL: str = "OP_ASSIGN_MUL"; break;
			case BinaryOperator::OP_ASSIGN_DIV: str = "OP_ASSIGN_DIV"; break;
			case BinaryOperator::OP_ASSIGN_MOD: str = "OP_ASSIGN_MOD"; break;
			case BinaryOperator::OP_ASSIGN_BIT_AND: str = "OP_ASSIGN_BIT_AND"; break;
			case BinaryOperator::OP_ASSIGN_BIT_OR: str = "OP_ASSIGN_BIT_OR"; break;
			case BinaryOperator::OP_ASSIGN_BIT_XOR: str = "OP_ASSIGN_BIT_XOR"; break;
			case BinaryOperator::OP_ASSIGN_LSHIFT: str = "OP_ASSIGN_LSHIFT"; break;
			case BinaryOperator::OP_ASSIGN_RSHIFT: str = "OP_ASSIGN_RSHIFT"; break;
			default: str = "UNKNOWN"; break;
		}
		return fmt::format_to(ctx.out(), "{}", str);
	}
};

template<>
/// @brief {fmt} specialization of UnaryOperator
struct fmt::formatter<colt::lang::UnaryOperator>
{
	template<typename ParseContext>
	constexpr auto parse(ParseContext& ctx) { return ctx.begin(); }

	template<typename FormatContext>
	auto format(const colt::lang::UnaryOperator& op, FormatContext& ctx)
	{
		using namespace colt::lang;
		const char* str;
		switch (op)
		{
			case UnaryOperator::OP_PRE_INCREMENT: str = "OP_PRE_INCREMENT"; break;
			case UnaryOperator::OP_POST_INCREMENT: str = "OP_POST_INCREMENT"; break;
			case UnaryOperator::OP_PRE_DECREMENT: str = "OP_PRE_DECREMENT"; break;
			case UnaryOperator::OP_POST_DECREMENT: str = "OP_POST_DECREMENT"; break;
			case UnaryOperator::OP_PLUS: str = "OP_PLUS"; break;
			case UnaryOperator::OP_NEGATE: str = "OP_NEGATE"; break;
			case UnaryOperator::OP_DEREFERENCE: str = "OP_DEREFERENCE"; break;
			case UnaryOperator::OP_ADDRESSOF: str = "OP_ADDRESSOF"; break;
			case UnaryOperator::OP_BOOL_NOT: str = "OP_BOOL_NOT"; break;
			case UnaryOperator::OP_BIT_NOT: str = "OP_BIT_NOT"; break;
			default: str = "UNKNOWN"; break;
		}
		return fmt::format_to(ctx.out(), "{}", str);
	}
};

#endif //!HG_COLT_OPERATORS