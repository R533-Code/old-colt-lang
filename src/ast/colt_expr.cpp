#include "colt_expr.h"
#include "context/colt_context.h"

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
			auto lhs_p = static_cast<const UnaryExpr*>(&lhs);
			auto rhs_p = static_cast<const UnaryExpr*>(&rhs);
			return lhs_p->get_operation() == rhs_p->get_operation()
				&& lhs_p->get_child() == rhs_p->get_child();
		}
		case Expr::EXPR_BINARY:
		{
			auto lhs_p = static_cast<const BinaryExpr*>(&lhs);
			auto rhs_p = static_cast<const BinaryExpr*>(&rhs);
			return lhs_p->get_operation() == rhs_p->get_operation()
				&& lhs_p->get_LHS() == rhs_p->get_LHS()
				&& lhs_p->get_RHS() == rhs_p->get_RHS();
		}
		case Expr::EXPR_CONVERT:
		{
			auto lhs_p = static_cast<const ConvertExpr*>(&lhs);
			auto rhs_p = static_cast<const ConvertExpr*>(&rhs);
			return lhs_p->get_child() == rhs_p->get_child();
		}
		case Expr::EXPR_VAR_DECL:
		{
			auto lhs_p = static_cast<const VariableDeclExpr*>(&lhs);
			auto rhs_p = static_cast<const VariableDeclExpr*>(&rhs);
			return lhs_p->get_name() == rhs_p->get_name()
				&& lhs_p->is_global() == rhs_p->is_global()
				&& lhs_p->get_value() == rhs_p->get_value();
		}
		case Expr::EXPR_VAR_READ:
		{
			auto lhs_p = static_cast<const VariableReadExpr*>(&lhs);
			auto rhs_p = static_cast<const VariableReadExpr*>(&rhs);
			return lhs_p->get_name() == rhs_p->get_name()
				&& lhs_p->unsafe_get_local_id() == rhs_p->unsafe_get_local_id();
		}
		case Expr::EXPR_VAR_WRITE:
		{
			auto lhs_p = static_cast<const VariableWriteExpr*>(&lhs);
			auto rhs_p = static_cast<const VariableWriteExpr*>(&rhs);
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
			auto lhs_p = static_cast<const ConditionExpr*>(&lhs);
			auto rhs_p = static_cast<const ConditionExpr*>(&rhs);
			return lhs_p->get_if_condition() == rhs_p->get_if_condition()
				&& lhs_p->get_if_statement() == rhs_p->get_if_statement()
				&& lhs_p->get_else_statement() == rhs_p->get_else_statement();
		}
		case Expr::EXPR_FN_RETURN:
		{
			auto lhs_p = static_cast<const FnReturnExpr*>(&lhs);
			auto rhs_p = static_cast<const FnReturnExpr*>(&rhs);
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

	bool operator==(const UniquePtr<Expr>& lhs, const UniquePtr<Expr>& rhs) noexcept
	{
		assert(lhs && rhs && "Invalid pointer to comparison!");
		return *lhs == *rhs;
	}
  
	PTR<Expr> LiteralExpr::CreateExpr(QWORD value, COLTContext& ctx) noexcept
  {
		return ctx.add_expr(make_unique<LiteralExpr>(value));
  }
}

namespace colt
{
	size_t hash(const lang::Expr& expr) noexcept
	{
		using namespace lang;

		switch (expr.classof())
		{
		case Expr::EXPR_LITERAL:
			return GetHash(static_cast<const LiteralExpr*>(&expr)->get_value().u64_v);
		
		case Expr::EXPR_UNARY:
		{
			auto lhs_p = static_cast<const UnaryExpr*>(&expr);
			return HashCombine(GetHash(lhs_p->get_operation()), GetHash(lhs_p->get_child()));
		}
		case Expr::EXPR_BINARY:
		{
			auto lhs_p = static_cast<const BinaryExpr*>(&expr);
			return HashCombine(GetHash(lhs_p->get_operation()),
				HashCombine(GetHash(lhs_p->get_LHS()), GetHash(lhs_p->get_RHS()))
			);
		}
		case Expr::EXPR_CONVERT:
		{
			auto lhs_p = static_cast<const ConvertExpr*>(&expr);
			return GetHash(lhs_p->get_child());
		}
		case Expr::EXPR_VAR_DECL:
		{
			auto lhs_p = static_cast<const VariableDeclExpr*>(&expr);
			return HashCombine(GetHash(lhs_p->get_name()),
				HashCombine(GetHash(lhs_p->is_global()), GetHash(lhs_p->get_value()))
			);
		}
		case Expr::EXPR_VAR_READ:
		{
			auto lhs_p = static_cast<const VariableReadExpr*>(&expr);
			return HashCombine(GetHash(lhs_p->get_name()), GetHash(lhs_p->unsafe_get_local_id()));
		}
		case Expr::EXPR_VAR_WRITE:
		{
			auto lhs_p = static_cast<const VariableWriteExpr*>(&expr);
			return HashCombine(GetHash(lhs_p->get_name()),
				HashCombine(GetHash(lhs_p->unsafe_get_local_id()), GetHash(lhs_p->get_value()))
			);
		}
		case Expr::EXPR_FN_DEF:
		{
			return 0;
		}
		case Expr::EXPR_CONDITION:
		{
			auto lhs_p = static_cast<const ConditionExpr*>(&expr);
			return HashCombine(GetHash(lhs_p->get_if_condition()),
				HashCombine(GetHash(lhs_p->get_if_statement()), GetHash(lhs_p->get_else_statement()))
			);
		}
		case Expr::EXPR_FN_RETURN:
		{
			auto lhs_p = static_cast<const FnReturnExpr*>(&expr);
			return GetHash(lhs_p->get_value());
		}
		case Expr::EXPR_SCOPE:
		{
			//TODO: add operators
			return 0;
		}

		case Expr::EXPR_FN_CALL:
		default: //Expr::EXPR_BASE is an error
			colt_unreachable("Invalid classof()");
		}
		return 0;
	}
}