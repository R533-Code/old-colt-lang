/** @file colt_operators.cpp
* Contains definition of functions declared in 'colt_operators.h'.
*/

#include "colt_operators.h"

namespace colt::lang
{
    UnaryOperator TokenToUnaryOperator(Token tkn, bool is_post) noexcept
    {
		switch (tkn)
		{
		case TKN_AND:
			return OP_ADDRESSOF;
		case TKN_STAR:
			return OP_DEREFERENCE;
		case TKN_PLUS:
			return OP_PLUS;
		case TKN_MINUS:
			return OP_NEGATE;
		case TKN_BANG:
			return OP_BOOL_NOT;
		case TKN_TILDE:
			return OP_BIT_NOT;
		case TKN_PLUS_PLUS:
			return is_post ? OP_POST_INCREMENT : OP_PRE_INCREMENT;
		case TKN_MINUS_MINUS:
			return is_post ? OP_POST_INCREMENT : OP_PRE_INCREMENT;
		default:
			colt_unreachable("Invalid Unary Operator!");
		}
    }
	
	BinaryOperator TokenToBinaryOperator(Token tkn) noexcept
	{
		//Valid binary operators are comprised in a range of tokens
		assert_true(static_cast<size_t>(tkn) < 29, "Invalid Binary Operator!");
		return static_cast<BinaryOperator>(tkn);
	}
}