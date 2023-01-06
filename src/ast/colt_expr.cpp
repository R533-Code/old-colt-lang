/** @file colt_expr.cpp
* Contains definition of functions declared in 'colt_expr.h'.
*/

#include "colt_expr.h"
#include "colt_context.h"

namespace colt::lang
{ 
  PTR<Expr> LiteralExpr::CreateExpr(QWORD value, PTR<const Type> type, const SourceCodeExprInfo& src_info, COLTContext& ctx) noexcept
  {
    return ctx.add_expr(make_unique<LiteralExpr>(value, type, src_info));
  }

  PTR<Expr> LiteralExpr::CreateExpr(QWORD value, Token tkn, const SourceCodeExprInfo& src_info, COLTContext& ctx) noexcept
  {
    assert_true(isLiteralToken(tkn), "Expected a Literal token!");
    PTR<const Type> type;
    switch (tkn)
    {
    break; case TKN_BOOL_L:   type = BuiltInType::CreateBool(true, ctx);
    break; case TKN_CHAR_L:   type = BuiltInType::CreateChar(true, ctx);
    break; case TKN_I8_L:     type = BuiltInType::CreateI8(true, ctx);
    break; case TKN_U8_L:     type = BuiltInType::CreateU8(true, ctx);
    break; case TKN_I16_L:    type = BuiltInType::CreateI16(true, ctx);
    break; case TKN_U16_L:    type = BuiltInType::CreateU16(true, ctx);
    break; case TKN_I32_L:    type = BuiltInType::CreateI32(true, ctx);
    break; case TKN_U32_L:    type = BuiltInType::CreateU32(true, ctx);
    break; case TKN_I64_L:    type = BuiltInType::CreateI64(true, ctx);
    break; case TKN_U64_L:    type = BuiltInType::CreateU64(true, ctx);
    break; case TKN_FLOAT_L:  type = BuiltInType::CreateF32(true, ctx);
    break; case TKN_DOUBLE_L: type = BuiltInType::CreateF64(true, ctx);
    break; case TKN_STRING_L: type = BuiltInType::CreateLString(ctx);
    break; default: colt_unreachable("Invalid Literal Token!");
    }
    return ctx.add_expr(make_unique<LiteralExpr>(value, type, src_info));
  }
  
  PTR<Expr> UnaryExpr::CreateExpr(PTR<const Type> type, Token tkn, PTR<Expr> child, const SourceCodeExprInfo& src_info, COLTContext& ctx) noexcept
  {
    return ctx.add_expr(make_unique<UnaryExpr>(type, tkn, child, src_info));
  }

  PTR<Expr> BinaryExpr::CreateExpr(PTR<const Type> type, PTR<Expr> lhs, Token op, PTR<Expr> rhs, const SourceCodeExprInfo& src_info, COLTContext& ctx) noexcept
  {
    return ctx.add_expr(make_unique<BinaryExpr>(type, lhs, op, rhs, src_info));
  }
  
  PTR<Expr> ConvertExpr::CreateExpr(PTR<const Type> type, PTR<Expr> to_convert, Token cnv, const SourceCodeExprInfo& src_info, COLTContext& ctx) noexcept
  {
    assert_true(cnv == TKN_KEYWORD_AS || cnv == TKN_KEYWORD_BIT_AS, "Expected a conversion token!");
    return ctx.add_expr(make_unique<ConvertExpr>(type, to_convert, 
      cnv == TKN_KEYWORD_AS ? CNV_AS : CNV_BIT_AS, src_info));
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
  
  PTR<Expr> VarWriteExpr::CreateExpr(PTR<const VarReadExpr> var, PTR<Expr> value, const SourceCodeExprInfo& src_info, COLTContext& ctx) noexcept
  {
    return ctx.add_expr(make_unique<VarWriteExpr>(var->get_type(), var->get_name(), value, var->unsafe_get_local_id(), src_info));
  }
  
  PTR<Expr> FnReturnExpr::CreateExpr(PTR<Expr> to_ret, const SourceCodeExprInfo& src_info, COLTContext& ctx) noexcept
  {
    return ctx.add_expr(make_unique<FnReturnExpr>(
      to_ret ? to_ret->get_type() : VoidType::CreateType(ctx), to_ret, src_info
      ));
  }
  
  PTR<Expr> FnDeclExpr::CreateExpr(PTR<const Type> type, StringView name, SmallVector<StringView, 4>&& arguments_name, bool is_extern, const SourceCodeExprInfo& src_info, COLTContext& ctx) noexcept
  {
    return ctx.add_expr(make_unique<FnDeclExpr>(
      type, name, std::move(arguments_name), is_extern, src_info
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
    return ctx.add_expr(make_unique<FnCallExpr>(
      decl, std::move(arguments), src_info
      ));
  }
  
  PTR<Expr> ScopeExpr::CreateExpr(Vector<PTR<Expr>>&& body, const SourceCodeExprInfo& src_info, COLTContext& ctx) noexcept
  {
    return ctx.add_expr(make_unique<ScopeExpr>(
      VoidType::CreateType(ctx), std::move(body), src_info
      ));
  }

  PTR<Expr> ScopeExpr::CreateExpr(std::initializer_list<PTR<Expr>> list, const SourceCodeExprInfo& src_info, COLTContext& ctx) noexcept
  {
    Vector<PTR<Expr>> body;
    for (auto i : list)
      body.push_back(i);
    return CreateExpr(std::move(body), src_info, ctx);
  }
  
  PTR<Expr> ConditionExpr::CreateExpr(PTR<Expr> if_cond, PTR<Expr> if_stmt, PTR<Expr> else_stmt, const SourceCodeExprInfo& src_info, COLTContext& ctx) noexcept
  {
    return ctx.add_expr(make_unique<ConditionExpr>(
      VoidType::CreateType(ctx), if_cond, if_stmt, else_stmt, src_info
      ));
  }

  PTR<Expr> WhileLoopExpr::CreateExpr(PTR<Expr> condition, PTR<Expr> body, const SourceCodeExprInfo& src_info, COLTContext& ctx) noexcept
  {
    return ctx.add_expr(make_unique<WhileLoopExpr>(
      VoidType::CreateType(ctx), condition, body, src_info
      ));
  }

  PTR<Expr> BreakContinueExpr::CreateExpr(bool is_break, const SourceCodeExprInfo& src_info, COLTContext& ctx) noexcept
  {
    return ctx.add_expr(make_unique<BreakContinueExpr>(
      VoidType::CreateType(ctx), is_break, src_info
      ));
  }
  
  PTR<Expr> ErrorExpr::CreateExpr(COLTContext& ctx) noexcept
  {
    return ctx.add_expr(make_unique<ErrorExpr>(
      ctx.add_type(make_unique<ErrorType>())
      ));
  }  
  
  PTR<Expr> NoOpExpr::CreateExpr(const SourceCodeExprInfo& src_info, COLTContext& ctx) noexcept
  {
    return ctx.add_expr(make_unique<NoOpExpr>(
      ctx.add_type(make_unique<VoidType>()), src_info
      ));
  }  
}