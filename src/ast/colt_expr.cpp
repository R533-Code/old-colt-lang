#include "colt_expr.h"

namespace colt::lang
{
  bool colt::lang::operator==(const Expr& lhs, const Expr& rhs) noexcept
  {
		//TODO: add type check for types
		if (lhs.classof() != rhs.classof())
			return false;

    switch (lhs.classof())
    {
		case Expr::EXPR_LITERAL:
		{
			//TODO: add type support comparison
			//return dyn_cast<const LiteralExpr*>(&lhs)->get_value() == dyn_cast<const LiteralExpr*>(&rhs)->get_value();
			return false;
		}			
		case Expr::EXPR_UNARY:
		{
			auto lhs_p = dyn_cast<const UnaryExpr*>(&lhs);
			auto rhs_p = dyn_cast<const UnaryExpr*>(&rhs);
			return lhs_p->get_operation() == rhs_p->get_operation()
				&& lhs_p->get_child() == rhs_p->get_child();
		}
		case Expr::EXPR_BINARY:
		{
			auto lhs_p = dyn_cast<const BinaryExpr*>(&lhs);
			auto rhs_p = dyn_cast<const BinaryExpr*>(&rhs);
			return lhs_p->get_operation() == rhs_p->get_operation()
				&& lhs_p->get_LHS() == rhs_p->get_LHS()
				&& lhs_p->get_RHS() == rhs_p->get_RHS();
		}
		case Expr::EXPR_CONVERT:
		{
			auto lhs_p = dyn_cast<const ConvertExpr*>(&lhs);
			auto rhs_p = dyn_cast<const ConvertExpr*>(&rhs);
			return lhs_p->get_child() == rhs_p->get_child();
		}
		case Expr::EXPR_VAR_DECL:
		{
			auto lhs_p = dyn_cast<const VariableDeclExpr*>(&lhs);
			auto rhs_p = dyn_cast<const VariableDeclExpr*>(&rhs);
			return lhs_p->get_name() == rhs_p->get_name()
				&& lhs_p->is_global() == rhs_p->is_global()
				&& lhs_p->get_value() == rhs_p->get_value();
		}
		case Expr::EXPR_VAR_READ:
		{
			auto lhs_p = dyn_cast<const VariableReadExpr*>(&lhs);
			auto rhs_p = dyn_cast<const VariableReadExpr*>(&rhs);
			return lhs_p->get_name() == rhs_p->get_name()
				&& lhs_p->unsafe_get_local_id() == rhs_p->unsafe_get_local_id();
		}
		case Expr::EXPR_VAR_WRITE:
		{
			auto lhs_p = dyn_cast<const VariableWriteExpr*>(&lhs);
			auto rhs_p = dyn_cast<const VariableWriteExpr*>(&rhs);
			return lhs_p->get_name() == rhs_p->get_name()
				&& lhs_p->unsafe_get_local_id() == rhs_p->unsafe_get_local_id()
				&& lhs_p->get_value() == rhs_p->get_value();
		}
		case Expr::EXPR_FN_DEF:
		{
			return false; //FIXME: maybe? overload resolution might help
		}
		case Expr::EXPR_CONDITION:
		{
			auto lhs_p = dyn_cast<const ConditionExpr*>(&lhs);
			auto rhs_p = dyn_cast<const ConditionExpr*>(&rhs);
			return lhs_p->get_if_condition() == rhs_p->get_if_condition()
				&& lhs_p->get_if_statement() == rhs_p->get_if_statement()
				&& lhs_p->get_else_statement() == rhs_p->get_else_statement();
		}
		case Expr::EXPR_FN_RETURN:
		{
			auto lhs_p = dyn_cast<const FnReturnExpr*>(&lhs);
			auto rhs_p = dyn_cast<const FnReturnExpr*>(&rhs);
			return lhs_p->get_value() == rhs_p->get_value();
		}
		case Expr::EXPR_SCOPE:
		{
			//TODO: add operators
		}

		case Expr::EXPR_FN_CALL:
		default: //Expr::EXPR_BASE is an error
			colt_unreachable("Invalid classof()");
    }
  }
}

