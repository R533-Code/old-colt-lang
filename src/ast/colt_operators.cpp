/** @file colt_operators.cpp
* Contains definition of functions declared in 'colt_operators.h'.
*/

#include "colt_operators.h"

namespace colt::lang
{
    UnaryOperator TokenToUnaryOperator(Token tkn) noexcept
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
      return UnaryOperator::OP_INCREMENT;
    case TKN_MINUS_MINUS:
      return UnaryOperator::OP_DECREMENT;
    default:
      colt_unreachable("Invalid Unary Operator!");
    }
    }
  
  BinaryOperator TokenToBinaryOperator(Token tkn) noexcept
  {
    //Valid binary operators are comprised in a range of tokens
    assert_true(static_cast<size_t>(tkn) < TKN_COMMA, "Invalid Binary Operator!");
    return static_cast<BinaryOperator>(tkn);
  }
  
  const char* BinaryOperatorToString(BinaryOperator tkn) noexcept
  {
    switch (tkn)
    {
    case BinaryOperator::OP_SUM:
      return "+";
    case BinaryOperator::OP_SUB:
      return "-";
    case BinaryOperator::OP_MUL:
      return "*";
    case BinaryOperator::OP_DIV:
      return "/";
    case BinaryOperator::OP_MOD:
      return "%";
    case BinaryOperator::OP_BIT_AND:
      return "&";
    case BinaryOperator::OP_BIT_OR:
      return "|";
    case BinaryOperator::OP_BIT_XOR:
      return "^";
    case BinaryOperator::OP_BIT_LSHIFT:
      return "<<";
    case BinaryOperator::OP_BIT_RSHIFT:
      return ">>";
    case BinaryOperator::OP_BOOL_AND:
      return "&&";
    case BinaryOperator::OP_BOOL_OR:
      return "||";
    case BinaryOperator::OP_LESS:
      return "<";
    case BinaryOperator::OP_LESS_EQUAL:
      return "<=";
    case BinaryOperator::OP_GREAT:
      return ">";
    case BinaryOperator::OP_GREAT_EQUAL:
      return ">=";
    case BinaryOperator::OP_NOT_EQUAL:
      return "!=";
    case BinaryOperator::OP_EQUAL:
      return "==";
    case BinaryOperator::OP_ASSIGN:
      return "=";
    case BinaryOperator::OP_ASSIGN_SUM:
      return "+=";
    case BinaryOperator::OP_ASSIGN_SUB:
      return "-=";
    case BinaryOperator::OP_ASSIGN_MUL:
      return "*=";
    case BinaryOperator::OP_ASSIGN_DIV:
      return "/=";
    case BinaryOperator::OP_ASSIGN_MOD:
      return "%=";
    case BinaryOperator::OP_ASSIGN_BIT_AND:
      return "&=";
    case BinaryOperator::OP_ASSIGN_BIT_OR:
      return "|=";
    case BinaryOperator::OP_ASSIGN_BIT_XOR:
      return "^=";
    case BinaryOperator::OP_ASSIGN_LSHIFT:
      return "<<=";
    case BinaryOperator::OP_ASSIGN_RSHIFT:
      return ">>=";
    default:
      colt_unreachable("Invalid operator!");
    }
  }
}