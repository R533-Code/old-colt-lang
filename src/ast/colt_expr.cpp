#include "colt_expr.h"
#include "context/colt_context.h"

namespace colt::lang
{
  bool operator==(const Expr& lhs, const Expr& rhs) noexcept
  {
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
      auto lhs_p = static_cast<const VarDeclExpr*>(&lhs);
      auto rhs_p = static_cast<const VarDeclExpr*>(&rhs);
      return lhs_p->get_name() == rhs_p->get_name()
        && lhs_p->is_global() == rhs_p->is_global()
        && lhs_p->get_value() == rhs_p->get_value();
    }
    case Expr::EXPR_VAR_READ:
    {
      auto lhs_p = static_cast<const VarReadExpr*>(&lhs);
      auto rhs_p = static_cast<const VarReadExpr*>(&rhs);
      return lhs_p->get_name() == rhs_p->get_name()
        && lhs_p->unsafe_get_local_id() == rhs_p->unsafe_get_local_id();
    }
    case Expr::EXPR_VAR_WRITE:
    {
      auto lhs_p = static_cast<const VarWriteExpr*>(&lhs);
      auto rhs_p = static_cast<const VarWriteExpr*>(&rhs);
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
      auto lhs_p = static_cast<const ScopeExpr*>(&lhs);
      auto rhs_p = static_cast<const ScopeExpr*>(&rhs);
      return lhs_p->get_body_array() == rhs_p->get_body_array();
    }
    case Expr::EXPR_ERROR:
    {
      return true;
    }

    case Expr::EXPR_FN_CALL:

    default: //Expr::EXPR_BASE is an error
      colt_unreachable("Invalid classof()");
    }
  }

  bool operator==(const UniquePtr<Expr>& lhs, const UniquePtr<Expr>& rhs) noexcept
  {
    if (lhs.get_ptr() == rhs.get_ptr())
      return true;
    if (!lhs || !rhs)
      return false;
    return *lhs == *rhs;
  }
  
  PTR<Expr> LiteralExpr::CreateExpr(QWORD value, PTR<const Type> type, COLTContext& ctx) noexcept
  {
    return ctx.add_expr(make_unique<LiteralExpr>(value, type));
  }
  
  PTR<Expr> UnaryExpr::CreateExpr(PTR<const Type> type, Token tkn, PTR<Expr> child, COLTContext& ctx) noexcept
  {
    return ctx.add_expr(make_unique<UnaryExpr>(type, tkn, child, false));
  }

  PTR<Expr> UnaryExpr::CreateExpr(PTR<const Type> type, Token tkn, bool is_post, PTR<Expr> child, COLTContext& ctx) noexcept
  {
    return ctx.add_expr(make_unique<UnaryExpr>(type, tkn, child, is_post));
  }
  
  PTR<Expr> BinaryExpr::CreateExpr(PTR<const Type> type, PTR<Expr> lhs, Token op, PTR<Expr> rhs, COLTContext& ctx) noexcept
  {
    return ctx.add_expr(make_unique<BinaryExpr>(type, lhs, op, rhs));
  }
  
  PTR<Expr> ConvertExpr::CreateExpr(PTR<const Type> type, PTR<Expr> to_convert, COLTContext& ctx) noexcept
  {
    return ctx.add_expr(make_unique<ConvertExpr>(type, to_convert));
  }
  
  PTR<Expr> VarDeclExpr::CreateExpr(PTR<const Type> type, StringView name, PTR<Expr> init_value, bool is_global, COLTContext& ctx) noexcept
  {
    return ctx.add_expr(make_unique<VarDeclExpr>(type, name, init_value, is_global));
  }
  
  PTR<Expr> VarReadExpr::CreateExpr(PTR<const Type> type, StringView name, u64 ID, COLTContext& ctx) noexcept
  {
    return ctx.add_expr(make_unique<VarReadExpr>(type, name, ID));
  }
  
  PTR<Expr> VarReadExpr::CreateExpr(PTR<const Type> type, StringView name, COLTContext& ctx) noexcept
  {
    return ctx.add_expr(make_unique<VarReadExpr>(type, name));
  }
  
  PTR<Expr> VarWriteExpr::CreateExpr(PTR<const Type> type, StringView name, PTR<Expr> value, u64 ID, COLTContext& ctx) noexcept
  {
    return ctx.add_expr(make_unique<VarWriteExpr>(type, name, value, ID));
  }
  
  PTR<Expr> VarWriteExpr::CreateExpr(PTR<const Type> type, StringView name, PTR<Expr> value, COLTContext& ctx) noexcept
  {
    return ctx.add_expr(make_unique<VarWriteExpr>(type, name, value));
  }
  
  PTR<Expr> FnReturnExpr::CreateExpr(PTR<Expr> to_ret, COLTContext& ctx) noexcept
  {
    return ctx.add_expr(make_unique<FnReturnExpr>(
      ctx.add_type(make_unique<VoidType>()), to_ret
      ));
  }
  
  PTR<Expr> FnDefExpr::CreateExpr(PTR<const Type> type, StringView name, SmallVector<StringView, 4>&& arguments_name, COLTContext& ctx) noexcept
  {
    return ctx.add_expr(make_unique<FnDefExpr>(type, name, std::move(arguments_name)));
  }
  
  PTR<Expr> FnDefExpr::CreateExpr(PTR<const Type> type, StringView name, SmallVector<StringView, 4>&& arguments_name, PTR<Expr> body, COLTContext& ctx) noexcept
  {
    return ctx.add_expr(make_unique<FnDefExpr>(type, name, std::move(arguments_name), body));
  }
  
  PTR<Expr> ScopeExpr::CreateExpr(Vector<PTR<Expr>>&& body, COLTContext& ctx) noexcept
  {
    return ctx.add_expr(make_unique<ScopeExpr>(
      ctx.add_type(make_unique<VoidType>()), std::move(body)
      ));
  }
  
  PTR<Expr> ConditionExpr::CreateExpr(PTR<Expr> if_cond, PTR<Expr> if_stmt, PTR<Expr> else_stmt, COLTContext& ctx) noexcept
  {    
    return ctx.add_expr(make_unique<ConditionExpr>(
      ctx.add_type(make_unique<VoidType>()), if_cond, if_stmt, else_stmt
      ));
  }
  
  PTR<Expr> ErrorExpr::CreateExpr(COLTContext& ctx) noexcept
  {
    return ctx.add_expr(make_unique<ErrorExpr>(
      ctx.add_type(make_unique<ErrorType>())
      ));
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
      auto lhs_p = static_cast<const VarDeclExpr*>(&expr);
      return HashCombine(GetHash(lhs_p->get_name()),
        HashCombine(GetHash(lhs_p->is_global()), GetHash(lhs_p->get_value()))
      );
    }
    case Expr::EXPR_VAR_READ:
    {
      auto lhs_p = static_cast<const VarReadExpr*>(&expr);
      return HashCombine(GetHash(lhs_p->get_name()), GetHash(lhs_p->unsafe_get_local_id()));
    }
    case Expr::EXPR_VAR_WRITE:
    {
      auto lhs_p = static_cast<const VarWriteExpr*>(&expr);
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
      return GetHash(static_cast<const ScopeExpr*>(&expr)->get_body_array());
    }
    case Expr::EXPR_ERROR:
    {
      return 18446744073709548283;
    }

    case Expr::EXPR_FN_CALL:
    default: //Expr::EXPR_BASE is an error
      colt_unreachable("Invalid classof()");
    }
    return 0;
  }
}