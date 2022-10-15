#include "colt_ast.h"

namespace colt::lang
{
  Optional<AST> CreateAST(StringView from, COLTContext& ctx) noexcept
  {
    details::ASTMaker ast = { from, ctx };
    if (ast.is_empty() || ast.get_error_count() != 0)
      return None;
    else
      return { InPlace, ast.steal_result(), ctx };
  }
}

namespace colt::lang::details
{
  u8 GetOpPrecedence(Token tkn) noexcept
  {
    static constexpr u8 operator_precedence_table[] =
    {
      10, 10, 11, 11, 11, // + - * / %
      6, 4, 5, 8, 8,  // & | ^ << >>
      3, 2,  // && ||
      9, 9, 9, 9, 10, 10, // < <= > >= != ==
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // = += -= *= /= %= &= |= ^= <<= >>=
      0, 0, 0, 0, 0 // , ; EOF ERROR )
    };
    assert_true(static_cast<size_t>(tkn) >= 0, "Token should be greater or equal to 0!");
    if (tkn < TKN_LEFT_PAREN)
      return operator_precedence_table[tkn];
    return 255;
  }

  bool isAssignmentToken(Token tkn) noexcept
  {
    return TKN_EQUAL_EQUAL < tkn && tkn < TKN_COMMA;
  }
  
  ASTMaker::SavedExprInfo::SavedExprInfo(ASTMaker& ast) noexcept
    : ast(ast), infos(ast.current_expr_info)
  {
    ast.current_expr_info = ast.get_expr_info();
  }
  
  ASTMaker::SavedExprInfo::~SavedExprInfo() noexcept
  {
    ast.current_expr_info = infos;
  }
  
  ASTMaker::SavedLocalState::SavedLocalState(ASTMaker& ast) noexcept
    : ast(ast), old_sz(ast.local_var_table.get_size()) {}
  
  ASTMaker::SavedLocalState::~SavedLocalState() noexcept
  {
    ast.local_var_table.pop_back_n(ast.local_var_table.get_size() - old_sz);
  }
  
  ASTMaker::ExprInfo ASTMaker::get_expr_info() const noexcept
  {
    auto scan_info = lexer.get_line_info();
    return { scan_info.line_nb, scan_info.line_strv, lexer.get_current_lexeme() };
  }
  
  ASTMaker::ASTMaker(StringView strv, COLTContext& ctx) noexcept
    : lexer(strv), ctx(ctx)
  {
    current_tkn = lexer.get_next_token();
    while (current_tkn != TKN_EOF)
    {
      expressions.push_back(parse_binary());
    }
  }
  
  PTR<Expr> ASTMaker::parse_primary() noexcept
  {
    //Save current expression state
    SavedExprInfo line_state = { *this };

    PTR<Expr> to_ret;
    switch (current_tkn)
    {
    break; case TKN_BOOL_L:
      to_ret = LiteralExpr::CreateExpr(lexer.get_parsed_value(), BuiltInType::CreateBool(true, ctx), ctx);
    break; case TKN_U8_L:
      to_ret = LiteralExpr::CreateExpr(lexer.get_parsed_value(), BuiltInType::CreateU8(true, ctx), ctx);
    break; case TKN_U16_L:
      to_ret = LiteralExpr::CreateExpr(lexer.get_parsed_value(), BuiltInType::CreateU16(true, ctx), ctx);
    break; case TKN_U32_L:
      to_ret = LiteralExpr::CreateExpr(lexer.get_parsed_value(), BuiltInType::CreateU32(true, ctx), ctx);
    break; case TKN_U64_L:
      to_ret = LiteralExpr::CreateExpr(lexer.get_parsed_value(), BuiltInType::CreateU64(true, ctx), ctx);
    break; case TKN_I8_L:
      to_ret = LiteralExpr::CreateExpr(lexer.get_parsed_value(), BuiltInType::CreateI8(true, ctx), ctx);
    break; case TKN_I16_L:
      to_ret = LiteralExpr::CreateExpr(lexer.get_parsed_value(), BuiltInType::CreateI16(true, ctx), ctx);
    break; case TKN_I32_L:
      to_ret = LiteralExpr::CreateExpr(lexer.get_parsed_value(), BuiltInType::CreateI32(true, ctx), ctx);
    break; case TKN_I64_L:
      to_ret = LiteralExpr::CreateExpr(lexer.get_parsed_value(), BuiltInType::CreateI64(true, ctx), ctx);
    break; case TKN_FLOAT_L:
      to_ret = LiteralExpr::CreateExpr(lexer.get_parsed_value(), BuiltInType::CreateF32(true, ctx), ctx);
    break; case TKN_DOUBLE_L:
      to_ret = LiteralExpr::CreateExpr(lexer.get_parsed_value(), BuiltInType::CreateF64(true, ctx), ctx);
    break; case TKN_STRING_L:      
      return nullptr;
    break; case TKN_CHAR_L:
      return nullptr;
      break;
    case TKN_AND:          // &(var)
    case TKN_PLUS_PLUS:    // ++(var)
    case TKN_MINUS_MINUS:  // --(var)
    case TKN_STAR:         // *(ptr)
    case TKN_TILDE:        // ~(any)
    case TKN_BANG:         // !(any)
    case TKN_MINUS:        // -(any)
    case TKN_PLUS:         // +(any)
      return parse_unary();
    case TKN_LEFT_PAREN:
      //return parse_parenthesis(&AST::parse_binary, static_cast<u8>(0));
      return nullptr;
    case TKN_IDENTIFIER:
      //return handle_variable_read(lexer.get_parsed_identifier());
      return nullptr;
    case TKN_ERROR: //Lexer will have generated an error
      consume_current_tkn(); //consume TKN_ERROR
      ++error_count;
      return nullptr;    // propagate the error
    default:
      gen_error_expr("Expected an expression!");
      return nullptr;
    }
    consume_current_tkn(); //consume the literal
    return to_ret;
  }

  PTR<Expr> ASTMaker::parse_binary(u8 precedence) noexcept
  {
    if (precedence == 255) //token was not an operator: error
    {
      gen_error_lexeme("Expected a binary operator!");
      return nullptr;//ErrorExpr::CreateExpr(ctx);
    }
    //Save current expression state
    SavedExprInfo line_state = { *this };


    PTR<Expr> lhs = parse_primary();
    //Save the current binary operators
    Token binary_op = current_tkn;

    //As assignment operators are right associative, they are handled
    //in a different function
    if (isAssignmentToken(binary_op))
      return nullptr;// parse_assignment(lhs);

    //The current operator's precedence
    u8 op_precedence = GetOpPrecedence(binary_op);

    while (op_precedence > precedence)
    {
      if (op_precedence == 255) //token was not an operator: error
      {
        gen_error_lexeme("Expected a ';'!");
        return nullptr;// ErrorExpr::CreateExpr(ctx);
      }

      //Consume the operator
      consume_current_tkn();
      //Recurse: 10 + 5 + 8 -> (10 + (5 + 8))
      PTR<Expr> rhs = parse_binary(GetOpPrecedence(binary_op));

      //Pratt's parsing, which allows operators priority
      lhs = BinaryExpr::CreateExpr(lhs->get_type(), lhs, binary_op, rhs, ctx);

      //Update the Token
      binary_op = current_tkn;
      //Update precedence
      op_precedence = GetOpPrecedence(binary_op);
    }

    return lhs;
  }

  PTR<Expr> ASTMaker::parse_unary() noexcept
  {
    //Save current expression state
    SavedExprInfo line_state = { *this };

    //Save the operator
    Token op = current_tkn;
    consume_current_tkn(); //consume the unary operator
    
    if (op == TKN_PLUS_PLUS) // +5 -> 5
      return parse_primary();

    //Parse the child expression -(5 + 8) -> PARENT -, CHILD (5 + 8)
    PTR<Expr> child = parse_primary();

    //No need to consume a Token as the previous call to parse_primary
    //already does
    return UnaryExpr::CreateExpr(child->get_type(), op, child, ctx);

  }
  
  void ASTMaker::panic_consume() noexcept
  {
    while (current_tkn != TKN_EOF && current_tkn != TKN_SEMICOLON)
      consume_current_tkn();
  }
}

