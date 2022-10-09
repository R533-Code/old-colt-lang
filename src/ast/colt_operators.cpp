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
			return UnaryOperator::OP_ADDRESSOF;
		case TKN_STAR:
			return UnaryOperator::OP_DEREFERENCE;
		case TKN_PLUS:
			return UnaryOperator::OP_PLUS;
		case TKN_MINUS:
			return UnaryOperator::OP_NEGATE;
		case TKN_BANG:
			return UnaryOperator::OP_BOOL_NOT;
		case TKN_TILDE:
			return UnaryOperator::OP_BIT_NOT;
		case TKN_PLUS_PLUS:
			return is_post ? UnaryOperator::OP_POST_INCREMENT : UnaryOperator::OP_PRE_INCREMENT;
		case TKN_MINUS_MINUS:
			return is_post ? UnaryOperator::OP_POST_INCREMENT : UnaryOperator::OP_PRE_INCREMENT;
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