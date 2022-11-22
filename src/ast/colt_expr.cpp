/** @file colt_expr.cpp
* Contains definition of functions declared in 'colt_expr.h'.
*/

#include "colt_expr.h"
#include "context/colt_context.h"

namespace colt::lang
{ 
  PTR<Expr> LiteralExpr::CreateExpr(QWORD value, PTR<const Type> type, const SourceCodeExprInfo& src_info, COLTContext& ctx) noexcept
  {
    return ctx.add_expr(make_unique<LiteralExpr>(value, type, src_info));
  }
  
  PTR<Expr> UnaryExpr::CreateExpr(PTR<const Type> type, Token tkn, PTR<Expr> child, const SourceCodeExprInfo& src_info, COLTContext& ctx) noexcept
  {
    return ctx.add_expr(make_unique<UnaryExpr>(type, tkn, child, false, src_info));
  }

  PTR<Expr> UnaryExpr::CreateExpr(PTR<const Type> type, Token tkn, bool is_post, PTR<Expr> child, const SourceCodeExprInfo& src_info, COLTContext& ctx) noexcept
  {
    return ctx.add_expr(make_unique<UnaryExpr>(type, tkn, child, is_post, src_info));
  }
  
  PTR<Expr> BinaryExpr::CreateExpr(PTR<const Type> type, PTR<Expr> lhs, Token op, PTR<Expr> rhs, const SourceCodeExprInfo& src_info, COLTContext& ctx) noexcept
  {
    return ctx.add_expr(make_unique<BinaryExpr>(type, lhs, op, rhs, src_info));
  }
  
  PTR<Expr> ConvertExpr::CreateExpr(PTR<const Type> type, PTR<Expr> to_convert, const SourceCodeExprInfo& src_info, COLTContext& ctx) noexcept
  {
    return ctx.add_expr(make_unique<ConvertExpr>(type, to_convert, src_info));
  }
  
  PTR<Expr> VarDeclExpr::CreateExpr(PTR<const Type> type, StringView name, PTR<Expr> init_value, bool is_global, const SourceCodeExprInfo& src_info, COLTContext& ctx) noexcept
  {
    return ctx.add_expr(make_unique<VarDeclExpr>(type, name, init_value, is_global, src_info));
  }
  
  PTR<Expr> VarReadExpr::CreateExpr(PTR<const Type> type, StringView name, u64 ID, const SourceCodeExprInfo& src_info, COLTContext& ctx) noexcept
  {
    return ctx.add_expr(make_unique<VarReadExpr>(type, name, ID, src_info));
  }
  
  PTR<Expr> VarReadExpr::CreateExpr(PTR<const Type> type, StringView name, const SourceCodeExprInfo& src_info, COLTContext& ctx) noexcept
  {
    return ctx.add_expr(make_unique<VarReadExpr>(type, name, src_info));
  }
  
  PTR<Expr> VarWriteExpr::CreateExpr(PTR<const Type> type, StringView name, PTR<Expr> value, u64 ID, const SourceCodeExprInfo& src_info, COLTContext& ctx) noexcept
  {
    return ctx.add_expr(make_unique<VarWriteExpr>(type, name, value, ID, src_info));
  }
  
  PTR<Expr> VarWriteExpr::CreateExpr(PTR<const Type> type, StringView name, PTR<Expr> value, const SourceCodeExprInfo& src_info, COLTContext& ctx) noexcept
  {
    return ctx.add_expr(make_unique<VarWriteExpr>(type, name, value, src_info));
  }
  
  PTR<Expr> FnReturnExpr::CreateExpr(PTR<Expr> to_ret, const SourceCodeExprInfo& src_info, COLTContext& ctx) noexcept
  {
    return ctx.add_expr(make_unique<FnReturnExpr>(
      to_ret->get_type(), to_ret, src_info
      ));
  }
  
  PTR<Expr> FnDeclExpr::CreateExpr(PTR<const Type> type, StringView name, SmallVector<StringView, 4>&& arguments_name, const SourceCodeExprInfo& src_info, COLTContext& ctx) noexcept
  {
    return ctx.add_expr(make_unique<FnDeclExpr>(
      type, name, std::move(arguments_name), src_info
      ));
  }

  PTR<Expr> FnDefExpr::CreateExpr(PTR<FnDeclExpr> decl, PTR<Expr> body, const SourceCodeExprInfo& src_info, COLTContext& ctx) noexcept
  {
    assert(is_a<FnDeclExpr>(static_cast<Expr*>(decl)));
    return ctx.add_expr(make_unique<FnDefExpr>(
      decl->get_type(), decl, body, src_info
      ));
  }
  
  PTR<Expr> FnDefExpr::CreateExpr(PTR<FnDeclExpr> decl, const SourceCodeExprInfo& src_info, COLTContext& ctx) noexcept
  {
    assert(is_a<FnDeclExpr>(static_cast<Expr*>(decl)));
    return ctx.add_expr(make_unique<FnDefExpr>(
      decl->get_type(), decl, nullptr, src_info
      ));
  }

  PTR<Expr> FnCallExpr::CreateExpr(PTR<const FnDeclExpr> decl, SmallVector<PTR<Expr>, 4>&& arguments, const SourceCodeExprInfo& src_info, COLTContext& ctx) noexcept
  {
    assert(decl->get_params_name().get_size() == arguments.get_size());
    return ctx.add_expr(make_unique<FnCallExpr>(
      decl, std::move(arguments), src_info, ctx
      ));
  }
  
  PTR<Expr> ScopeExpr::CreateExpr(Vector<PTR<Expr>>&& body, const SourceCodeExprInfo& src_info, COLTContext& ctx) noexcept
  {
    return ctx.add_expr(make_unique<ScopeExpr>(
      ctx.add_type(make_unique<VoidType>()), std::move(body), src_info
      ));
  }
  
  PTR<Expr> ConditionExpr::CreateExpr(PTR<Expr> if_cond, PTR<Expr> if_stmt, PTR<Expr> else_stmt, const SourceCodeExprInfo& src_info, COLTContext& ctx) noexcept
  {
    return ctx.add_expr(make_unique<ConditionExpr>(
      ctx.add_type(make_unique<VoidType>()), if_cond, if_stmt, else_stmt, src_info
      ));
  }
  
  PTR<Expr> ErrorExpr::CreateExpr(COLTContext& ctx) noexcept
  {
    return ctx.add_expr(make_unique<ErrorExpr>(
      ctx.add_type(make_unique<ErrorType>())
      ));
  }
}