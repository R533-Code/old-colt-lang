/** @file colt_ast.cpp
* Contains definition of functions declared in 'colt_ast.h'.
*/

#include "colt_ast.h"

namespace colt::lang
{
  Expected<AST, u32> CreateAST(StringView from, COLTContext& ctx) noexcept
  {
    details::ASTMaker ast = { from, ctx };
    if (ast.is_empty() || ast.get_error_count() != 0)
      return { Error, ast.get_error_count() };
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
      0, 0, 0, 0, 0, 0, 0, 0, 0 // , ; EOF ERROR ) ( : } {
    };
    assert_true(static_cast<size_t>(tkn) >= 0, "Token should be greater or equal to 0!");
    if (tkn < TKN_MINUS_GREAT)
      return operator_precedence_table[tkn];
    return 255;
  }

  bool isAssignmentToken(Token tkn) noexcept
  {
    return TKN_EQUAL_EQUAL < tkn && tkn < TKN_COMMA;
  }

  SourceCodeExprInfo ConcatInfo(const SourceCodeExprInfo& lhs, const SourceCodeExprInfo& rhs) noexcept
  {
    return SourceCodeExprInfo{ lhs.line_begin, rhs.line_end,
      StringView{lhs.lines.begin(), rhs.lines.end()},
      StringView{lhs.expression.begin(), rhs.expression.end()},
    };
  }
  
  ASTMaker::SavedExprInfo::SavedExprInfo(ASTMaker& ast) noexcept
    : ast(ast), infos(ast.current_lexeme_info)
  {
    ast.current_lexeme_info = ast.get_expr_info();
  }
  
  ASTMaker::SavedExprInfo::~SavedExprInfo() noexcept
  {
    ast.current_lexeme_info = infos;
  }

  SourceCodeExprInfo ASTMaker::SavedExprInfo::to_src_info() const noexcept
  {
    return SourceCodeExprInfo{ as<u32>(ast.current_lexeme_info.line_nb), as<u32>(ast.last_lexeme_info.line_nb),
      StringView{ast.current_lexeme_info.line_strv.begin(), ast.last_lexeme_info.line_strv.end()},
      StringView{ast.current_lexeme_info.expression.begin(), ast.last_lexeme_info.expression.end()},
    };
  }
  
  ASTMaker::SavedLocalState::SavedLocalState(ASTMaker& ast) noexcept
    : ast(ast), old_sz(ast.local_var_table.get_size()) {}
  
  ASTMaker::SavedLocalState::~SavedLocalState() noexcept
  {
    ast.local_var_table.pop_back_n(ast.local_var_table.get_size() - old_sz);
  }
  
  ASTMaker::SourceCodeLexemeInfo ASTMaker::get_expr_info() const noexcept
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
      expressions.push_back(parse_global_declaration());
    }
  }
  
  void ASTMaker::consume_current_tkn() noexcept
  {
    last_lexeme_info = get_expr_info();
    current_tkn = lexer.get_next_token();
  }

  PTR<Expr> ASTMaker::parse_primary() noexcept
  {
    //Save current expression state
    SavedExprInfo line_state = { *this };

    PTR<Expr> to_ret;    

    switch (current_tkn)
    {
    break; case TKN_BOOL_L:
      consume_current_tkn();
      to_ret = LiteralExpr::CreateExpr(lexer.get_parsed_value(), BuiltInType::CreateBool(true, ctx),
        line_state.to_src_info(), ctx);
    break; case TKN_U8_L:
      consume_current_tkn();
      to_ret = LiteralExpr::CreateExpr(lexer.get_parsed_value(), BuiltInType::CreateU8(true, ctx),
        line_state.to_src_info(), ctx);
    break; case TKN_U16_L:
      consume_current_tkn();
      to_ret = LiteralExpr::CreateExpr(lexer.get_parsed_value(), BuiltInType::CreateU16(true, ctx),
        line_state.to_src_info(), ctx);
    break; case TKN_U32_L:
      consume_current_tkn();
      to_ret = LiteralExpr::CreateExpr(lexer.get_parsed_value(), BuiltInType::CreateU32(true, ctx),
        line_state.to_src_info(), ctx);
    break; case TKN_U64_L:
      consume_current_tkn();
      to_ret = LiteralExpr::CreateExpr(lexer.get_parsed_value(), BuiltInType::CreateU64(true, ctx),
        line_state.to_src_info(), ctx);
    break; case TKN_I8_L:
      consume_current_tkn();
      to_ret = LiteralExpr::CreateExpr(lexer.get_parsed_value(), BuiltInType::CreateI8(true, ctx),
        line_state.to_src_info(), ctx);
    break; case TKN_I16_L:
      consume_current_tkn();
      to_ret = LiteralExpr::CreateExpr(lexer.get_parsed_value(), BuiltInType::CreateI16(true, ctx),
        line_state.to_src_info(), ctx);
    break; case TKN_I32_L:
      consume_current_tkn();
      to_ret = LiteralExpr::CreateExpr(lexer.get_parsed_value(), BuiltInType::CreateI32(true, ctx),
        line_state.to_src_info(), ctx);
    break; case TKN_I64_L:
      consume_current_tkn();
      to_ret = LiteralExpr::CreateExpr(lexer.get_parsed_value(), BuiltInType::CreateI64(true, ctx),
        line_state.to_src_info(), ctx);
    break; case TKN_FLOAT_L:
      consume_current_tkn();
      to_ret = LiteralExpr::CreateExpr(lexer.get_parsed_value(), BuiltInType::CreateF32(true, ctx),
        line_state.to_src_info(), ctx);
    break; case TKN_DOUBLE_L:
      consume_current_tkn();
      to_ret = LiteralExpr::CreateExpr(lexer.get_parsed_value(), BuiltInType::CreateF64(true, ctx),
        line_state.to_src_info(), ctx);
    break; case TKN_STRING_L:
      consume_current_tkn();
      to_ret = ErrorExpr::CreateExpr(ctx);
    break; case TKN_CHAR_L:
      consume_current_tkn();
      to_ret = ErrorExpr::CreateExpr(ctx);

      break;
    case TKN_AND:          // &(var)
    case TKN_PLUS_PLUS:    // ++(var)
    case TKN_MINUS_MINUS:  // --(var)
    case TKN_STAR:         // *(ptr)
    case TKN_TILDE:        // ~(any)
    case TKN_BANG:         // !(any)
    case TKN_MINUS:        // -(any)
    case TKN_PLUS:         // +(any)
      to_ret = parse_unary();    
    
    break; case TKN_IDENTIFIER:
      to_ret = parse_identifier();

    break; case TKN_ERROR: //Lexer will have generated an error
      ++error_count;
      to_ret = ErrorExpr::CreateExpr(ctx);
      
    break; case TKN_LEFT_PAREN:
      to_ret = parse_parenthesis(&ASTMaker::parse_binary, static_cast<u8>(0));

    break; default:
      gen_error_expr("Expected an expression!");
      return ErrorExpr::CreateExpr(ctx);
    }

    //Post-unary operators
    if (current_tkn == TKN_PLUS_PLUS || current_tkn == TKN_MINUS_MINUS)
    {
      to_ret = UnaryExpr::CreateExpr(to_ret->get_type(), current_tkn, true, to_ret, line_state.to_src_info(), ctx);
      consume_current_tkn(); //consume the post unary operator
    }
    return to_ret;
  }

  PTR<Expr> ASTMaker::parse_binary(u8 precedence) noexcept
  {
    if (precedence == 255) //token was not an operator: error
    {
      gen_error_lexeme("Expected a binary operator!");
      return ErrorExpr::CreateExpr(ctx);
    }
    //Save current expression state
    SavedExprInfo line_state = { *this };


    PTR<Expr> lhs = parse_primary();
    //Save the current binary operators
    Token binary_op = current_tkn;

    //As assignment operators are right associative, they are handled
    //in a different function
    if (isAssignmentToken(binary_op))
      return parse_assignment(lhs);

    //The current operator's precedence
    u8 op_precedence = GetOpPrecedence(binary_op);

    while (op_precedence > precedence)
    {
      if (op_precedence == 255) //token was not an operator: error
      {
        gen_error_lexeme("Expected a ';'!");
        return ErrorExpr::CreateExpr(ctx);
      }

      //Consume the operator
      consume_current_tkn();
      //Recurse: 10 + 5 + 8 -> (10 + (5 + 8))
      PTR<Expr> rhs = parse_binary(GetOpPrecedence(binary_op));

      //Pratt's parsing, which allows operators priority
      lhs = BinaryExpr::CreateExpr(lhs->get_type(), lhs, binary_op, rhs,
        line_state.to_src_info(), ctx);

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
    return UnaryExpr::CreateExpr(child->get_type(), op, child,
      line_state.to_src_info(), ctx);
  }

  PTR<Expr> ASTMaker::parse_global_declaration() noexcept
  {
    //Function:
    // fn NAME \( (TYPE NAME,)* (TYPE NAME)? \) -> TYPE PARSE_SCOPE
    if (current_tkn == TKN_KEYWORD_FN)
    {
      auto expr = parse_fn_decl();
      if (is_a<FnDefExpr>(expr)) //add to the global table
        global_map.insert(as<PTR<FnDefExpr>>(expr)->get_name(), expr);
      return expr;
    }
    //Global Variable:
    //var NAME = VALUE;
    else
      return parse_variable_decl(true);
  }

  PTR<Expr> ASTMaker::parse_fn_decl() noexcept
  {
    SavedExprInfo line_state = { *this };

    assert(current_tkn == TKN_KEYWORD_FN);
    consume_current_tkn();
    auto fn_name = lexer.get_parsed_identifier();
    check_and_consume(TKN_IDENTIFIER, "Expected an identifier, not '{}'!", fn_name);
    check_and_consume(TKN_LEFT_PAREN, "Expected a '('!");
    
    SmallVector<PTR<const Type>, 4> args_type;
    SmallVector<StringView, 4> args_name;
    while (current_tkn != TKN_EOF && current_tkn != TKN_RIGHT_PAREN)
    {
      args_type.push_back(parse_typename());
      auto arg_name = lexer.get_parsed_identifier();
      if (check_and_consume(TKN_IDENTIFIER, "Expected an identifier!"))
      {
        panic_consume_rparen();
        break;
      }
      
      if (args_name.to_view().contains(arg_name))
      {
        auto line_info = lexer.get_line_info();
        GenerateError(line_info.line_nb, line_info.line_strv, arg_name, "Cannot have parameters of same name '{}'!", arg_name);

        ++error_count;
        panic_consume_rparen();
        break;
      }
      args_name.push_back(arg_name);

      if (current_tkn == TKN_RIGHT_PAREN)
        break;
      if (check_and_consume(TKN_COMMA, "Expected a ','!"))
      {
        panic_consume_rparen();
        break;
      }
    }

    check_and_consume(TKN_RIGHT_PAREN, "Expected a ')'!");
    check_and_consume(TKN_MINUS_GREAT, "Expected a '->'!");
    //The return type of the function
    PTR<const Type> return_t = parse_typename();

    PTR<const Type> fn_ptr_t = FnType::CreateFn(return_t, std::move(args_type), ctx);
    PTR<FnDeclExpr> declaration = as<PTR<FnDeclExpr>>(FnDeclExpr::CreateExpr(fn_ptr_t, fn_name, std::move(args_name), line_state.to_src_info(), ctx));

    //Set the current function being parsed
    current_function = declaration;
    //And reset it on scope exit
    ON_EXIT{ current_function = nullptr; };

    if (is_valid_scope_begin())
      return FnDefExpr::CreateExpr(declaration, parse_scope(), line_state.to_src_info(), ctx);
    check_and_consume(TKN_SEMICOLON, "Expected a ';'!");
    return FnDefExpr::CreateExpr(declaration, line_state.to_src_info(), ctx);
  }

  PTR<Expr> ASTMaker::parse_scope(bool one_expr) noexcept
  {
    if (current_tkn == TKN_COLON && one_expr)
    {
      consume_current_tkn(); // :
      return parse_statement();
    }
    else if (current_tkn == TKN_LEFT_CURLY)
    {
      //Save '{' informations
      auto lexeme_info = get_expr_info();
      consume_current_tkn(); // {
      
      Vector<PTR<Expr>> statements = {};
      while (current_tkn != TKN_RIGHT_CURLY && current_tkn != TKN_EOF)
        statements.push_back(parse_statement());
      
      if (current_tkn != TKN_RIGHT_CURLY)
      {
        GenerateError(lexeme_info.line_nb, lexeme_info.line_strv, lexeme_info.expression, "Unclosed curly bracket delimiter!");
        ++error_count;
      }
      else
        consume_current_tkn();
    }
    else
      gen_error_lexeme("Expected a scope!");
    return ErrorExpr::CreateExpr(ctx);
  }

  PTR<Expr> ASTMaker::parse_statement() noexcept
  {
    //TODO: add if/for/while handling
    switch (current_tkn)
    {
    case TKN_KEYWORD_VAR:
      return parse_variable_decl(false);
    case TKN_LEFT_CURLY:
      return parse_scope(false);
    case TKN_KEYWORD_IF:
      return parse_condition();
    case TKN_SEMICOLON:
      gen_error_lexeme("Expected a statement!");
      return ErrorExpr::CreateExpr(ctx);
    default: break;
    }
    //Save the error count
    auto old_count = error_count;
    auto to_ret = parse_binary();
    //If an error was generated by parse_binary(), the semicolon
    //would be consumed, so do not check for it
    if (old_count == error_count || current_tkn == TKN_SEMICOLON)
      check_and_consume(TKN_SEMICOLON, "Expected a ';'!");
    return to_ret;
  }

  PTR<Expr> ASTMaker::parse_condition() noexcept
  {
    return PTR<Expr>();
  }

  PTR<Expr> ASTMaker::parse_variable_decl(bool is_global) noexcept
  {
    SavedExprInfo line_state = { *this };

    if (check_and_consume(TKN_KEYWORD_VAR, "Expected a variable declaration!"))
      return ErrorExpr::CreateExpr(ctx);
    if (check_and_consume(TKN_IDENTIFIER, "Expected an identifier!"))
      return ErrorExpr::CreateExpr(ctx);

    StringView var_name = lexer.get_parsed_identifier();

    PTR<const Type> var_type = nullptr;
    if (current_tkn == TKN_COLON)
    {
      consume_current_tkn();
      var_type = parse_typename();
    }
    
    PTR<Expr> var_init = nullptr;
    if (current_tkn != TKN_SEMICOLON)
    {
      if (check_and_consume(TKN_EQUAL, "Expected a '='!"))
        return ErrorExpr::CreateExpr(ctx);
      var_init = parse_binary();
    }
    else if (var_type == nullptr)
    {
      gen_error_expr("An uninitialized variable should specify its type!");
      return ErrorExpr::CreateExpr(ctx);
    }

    //If the type is not explicit, deduce it from left hand side
    //else convert left hand side to explicit type
    if (var_type == nullptr)
      var_type = var_init->get_type();
    else
      var_init = ConvertExpr::CreateExpr(var_type, var_init,
        line_state.to_src_info(), ctx);
    
    if (check_and_consume(TKN_SEMICOLON, "Expected a ';'!"))
      return ErrorExpr::CreateExpr(ctx);      

    if (is_global)
    {
      auto var_expr = VarDeclExpr::CreateExpr(var_type, var_name, var_init, true,
        line_state.to_src_info(), ctx);
      global_map.insert(var_name, var_expr);
      return var_expr;
    }
    
    local_var_table.push_back({ var_name, var_type });    
    return VarDeclExpr::CreateExpr(var_type, var_name, var_init, false,
      line_state.to_src_info(), ctx);
  }

  PTR<Expr> ASTMaker::parse_assignment(PTR<Expr> lhs) noexcept
  {
    //Save current expression state
    SavedExprInfo line_state = { *this };

    Token assignment_tkn = current_tkn;
    PTR<Expr> rhs = parse_binary();

    if (!is_a<VarReadExpr>(lhs))
    {
      gen_error_expr("Left hand side of an assignment should be a variable!");
      return ErrorExpr::CreateExpr(ctx);
    }

    if (assignment_tkn == TKN_EQUAL)
      return VarWriteExpr::CreateExpr(lhs->get_type(), as<VarReadExpr*>(lhs)->get_name(), rhs,
        line_state.to_src_info(), ctx);

    //Expands VAR += VALUE as VAR = VAR + VALUE
    switch (assignment_tkn)
    {    
    case colt::lang::TKN_PLUS_EQUAL:
      return VarWriteExpr::CreateExpr(lhs->get_type(), as<VarReadExpr*>(lhs)->get_name(),
        BinaryExpr::CreateExpr(lhs->get_type(), lhs, TKN_PLUS, rhs, line_state.to_src_info(), ctx), line_state.to_src_info(), ctx);
    case colt::lang::TKN_MINUS_EQUAL:
      return VarWriteExpr::CreateExpr(lhs->get_type(), as<VarReadExpr*>(lhs)->get_name(),
        BinaryExpr::CreateExpr(lhs->get_type(), lhs, TKN_MINUS, rhs, line_state.to_src_info(), ctx), line_state.to_src_info(), ctx);
    case colt::lang::TKN_STAR_EQUAL:
      return VarWriteExpr::CreateExpr(lhs->get_type(), as<VarReadExpr*>(lhs)->get_name(),
        BinaryExpr::CreateExpr(lhs->get_type(), lhs, TKN_STAR, rhs, line_state.to_src_info(), ctx), line_state.to_src_info(), ctx);
    case colt::lang::TKN_SLASH_EQUAL:
      return VarWriteExpr::CreateExpr(lhs->get_type(), as<VarReadExpr*>(lhs)->get_name(),
        BinaryExpr::CreateExpr(lhs->get_type(), lhs, TKN_SLASH, rhs, line_state.to_src_info(), ctx), line_state.to_src_info(), ctx);
    case colt::lang::TKN_PERCENT_EQUAL:
      return VarWriteExpr::CreateExpr(lhs->get_type(), as<VarReadExpr*>(lhs)->get_name(),
        BinaryExpr::CreateExpr(lhs->get_type(), lhs, TKN_PERCENT, rhs, line_state.to_src_info(), ctx), line_state.to_src_info(), ctx);
    case colt::lang::TKN_AND_EQUAL:
      return VarWriteExpr::CreateExpr(lhs->get_type(), as<VarReadExpr*>(lhs)->get_name(),
        BinaryExpr::CreateExpr(lhs->get_type(), lhs, TKN_AND, rhs, line_state.to_src_info(), ctx), line_state.to_src_info(), ctx);
    case colt::lang::TKN_OR_EQUAL:
      return VarWriteExpr::CreateExpr(lhs->get_type(), as<VarReadExpr*>(lhs)->get_name(),
        BinaryExpr::CreateExpr(lhs->get_type(), lhs, TKN_OR, rhs, line_state.to_src_info(), ctx), line_state.to_src_info(), ctx);
    case colt::lang::TKN_CARET_EQUAL:
      return VarWriteExpr::CreateExpr(lhs->get_type(), as<VarReadExpr*>(lhs)->get_name(),
        BinaryExpr::CreateExpr(lhs->get_type(), lhs, TKN_CARET, rhs, line_state.to_src_info(), ctx), line_state.to_src_info(), ctx);
    case colt::lang::TKN_LESS_LESS_EQUAL:
      return VarWriteExpr::CreateExpr(lhs->get_type(), as<VarReadExpr*>(lhs)->get_name(),
        BinaryExpr::CreateExpr(lhs->get_type(), lhs, TKN_LESS_LESS, rhs, line_state.to_src_info(), ctx), line_state.to_src_info(), ctx);
    case colt::lang::TKN_GREAT_GREAT_EQUAL:
      return VarWriteExpr::CreateExpr(lhs->get_type(), as<VarReadExpr*>(lhs)->get_name(),
        BinaryExpr::CreateExpr(lhs->get_type(), lhs, TKN_GREAT_GREAT, rhs, line_state.to_src_info(), ctx), line_state.to_src_info(), ctx);
    default:
      colt_unreachable("Invalid assignment token!");
    }
  }

  PTR<const Type> ASTMaker::parse_typename() noexcept
  {
    //Save current expression state
    SavedExprInfo line_state = { *this };

    if (current_tkn == TKN_KEYWORD_TYPEOF) //typeof(10 + 5) -> get_type of (10 + 5)
    {
      consume_current_tkn();
      return parse_parenthesis(&ASTMaker::parse_binary, static_cast<u8>(0))->get_type();
    }
    
    bool is_mut = false;
    if (current_tkn == TKN_KEYWORD_MUT) // mut TYPE
    {
      is_mut = true;
      consume_current_tkn();
    }

    //Consume the typename token
    ON_EXIT{ consume_current_tkn(); };

    switch (current_tkn)
    {
    case TKN_KEYWORD_VOID:
    {
      if (is_mut)
        gen_error_expr("'void' typename cannot be marked as mutable!");
      return VoidType::CreateType(ctx);
    }
    case TKN_KEYWORD_BOOL:
      return BuiltInType::CreateBool(is_mut, ctx);
    case TKN_KEYWORD_CHAR:
      //TODO: add
      colt_unreachable("not implemented");
      //return BuiltInType::CreateChar(is_mut, ctx);
    case TKN_KEYWORD_I8:
      return BuiltInType::CreateI8(is_mut, ctx);
    case TKN_KEYWORD_U8:
      return BuiltInType::CreateU8(is_mut, ctx);
    case TKN_KEYWORD_I16:
      return BuiltInType::CreateI16(is_mut, ctx);
    case TKN_KEYWORD_U16:
      return BuiltInType::CreateU16(is_mut, ctx);
    case TKN_KEYWORD_I32:
      return BuiltInType::CreateI32(is_mut, ctx);
    case TKN_KEYWORD_U32:
      return BuiltInType::CreateU32(is_mut, ctx);
    case TKN_KEYWORD_I64:
      return BuiltInType::CreateI64(is_mut, ctx);
    case TKN_KEYWORD_U64:
      return BuiltInType::CreateU64(is_mut, ctx);
    case TKN_KEYWORD_FLOAT:
      return BuiltInType::CreateF32(is_mut, ctx);
    case TKN_KEYWORD_DOUBLE:
      return BuiltInType::CreateF64(is_mut, ctx);
    case TKN_KEYWORD_LSTRING:
      //TODO: add
      colt_unreachable("not implemented");
    case TKN_KEYWORD_PTR:
    {
      if (!check_and_consume(TKN_LESS, "Expected a '<'!"))
      {
        PTR<const Type> ptr_to = parse_typename();
        if (current_tkn == TKN_GREAT_GREAT) // '>>' is parsed as '>' '>'
          current_tkn = TKN_GREAT;
        else if (!check_and_consume(TKN_GREAT, "Expected a '>'!"))
          return PtrType::CreatePtr(is_mut, ptr_to, ctx);
      }
    }
    break;
    case TKN_IDENTIFIER:
      //TODO: add
      colt_unreachable("not implemented");
    default:
      gen_error_expr("Expected a typename!");
    }
    //return an error
    return ErrorType::CreateType(ctx);
  }

  PTR<Expr> ASTMaker::parse_identifier() noexcept
  {
    assert(current_tkn == TKN_IDENTIFIER);
    
    SavedExprInfo line_state = { *this };
    
    StringView identifier = lexer.get_parsed_identifier();
    consume_current_tkn(); // consume identifier
    if (current_tkn == TKN_LEFT_PAREN) // function call
      return parse_function_call(identifier, line_state);

    if (current_function != nullptr) //if parsing a function
    {
      //Search in local variables of function
      for (i64 i = as<i64>(local_var_table.get_size()) - 1; i >= 0; i--)
      {
        if (local_var_table[i].first == identifier)
          return VarReadExpr::CreateExpr(local_var_table[i].second, identifier, i,
            line_state.to_src_info(), ctx);
      }
      auto fn_param = current_function->get_params_name();
      //Search in arguments of function
      for (size_t i = 0; i < fn_param.get_size(); i++)
      {
        if (fn_param[i] == identifier)
          return VarReadExpr::CreateExpr(current_function->get_params_type()[i], identifier,
            line_state.to_src_info(), ctx);
      }
    }
    gen_error_expr("Variable of name '{}' does not exist!", identifier);
    //TODO: Search global variables
    return ErrorExpr::CreateExpr(ctx);
  }

  PTR<Expr> ASTMaker::parse_function_call(StringView identifier, const SavedExprInfo& line_state) noexcept
  {
    assert(current_tkn == TKN_LEFT_PAREN);

    SourceCodeExprInfo identifier_location = line_state.to_src_info();

    SmallVector<PTR<Expr>, 4> arguments;
    parse_parenthesis(&ASTMaker::parse_function_call_arguments, arguments);

    if (auto found = global_map.find(identifier))
    {
      if (is_a<FnDefExpr>(found->second))
      {
        auto decl = as<PTR<FnDefExpr>>(found->second)->get_fn_decl();
        if (validate_fn_call(arguments, decl, identifier, identifier_location))
          return ErrorExpr::CreateExpr(ctx);        
        return FnCallExpr::CreateExpr(decl, std::move(arguments),
            line_state.to_src_info(), ctx);
      }
      else // and return an ErrorExpr
        gen_error_src_info(identifier_location, "'{}' is not a function!", identifier);
    }
    else
      gen_error_src_info(identifier_location, "Function of name '{}' does not exist!", identifier);
    return ErrorExpr::CreateExpr(ctx);
  }

  void ASTMaker::parse_function_call_arguments(SmallVector<PTR<Expr>, 4>& arguments) noexcept
  {
    if (current_tkn != TKN_RIGHT_PAREN)
      arguments.push_back(parse_binary());
    while (current_tkn != TKN_RIGHT_PAREN)
    {
      if (check_and_consume(TKN_COMMA, "Expected a ')'!"))
        break;
      arguments.push_back(parse_binary());
    }
  }

  bool ASTMaker::validate_fn_call(const SmallVector<PTR<Expr>, 4>& arguments, PTR<const FnDeclExpr> decl, StringView identifier, const SourceCodeExprInfo& info) noexcept
  {
    if (arguments.get_size() != decl->get_params_name().get_size())
    {
      gen_error_src_info(info, "Function '{}' expects {} argument{} not {}!", identifier,
        decl->get_params_name().get_size(), decl->get_params_name().get_size() == 1 ? "," : "s,", arguments.get_size());
      return false;
    }
    bool ret = true;
    for (size_t i = 0; i < arguments.get_size(); i++)
    {
      if (arguments[i]->get_type() != decl->get_params_type()[i])
      {
        //TODO: checking for pointer types
        gen_error_src_info(arguments[i]->get_src_code(), "Type of argument does not match that of declaration!");
        ret = false;
      }
    }
    return ret;
  }
  
  void ASTMaker::panic_consume() noexcept
  {
    while (current_tkn != TKN_EOF && current_tkn != TKN_SEMICOLON
      && current_tkn != TKN_RIGHT_CURLY && current_tkn != TKN_RIGHT_PAREN)
      consume_current_tkn();
    if (current_tkn == TKN_SEMICOLON)
      consume_current_tkn();
  }
  
  void ASTMaker::panic_consume_rparen() noexcept
  {
    while (current_tkn != TKN_RIGHT_PAREN && current_tkn != TKN_EOF && current_tkn != TKN_SEMICOLON)
      consume_current_tkn();
    if (current_tkn == TKN_SEMICOLON)
      consume_current_tkn();
  }
}

