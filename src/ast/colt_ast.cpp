/** @file colt_ast.cpp
* Contains definition of functions declared in 'colt_ast.h'.
*/

#include "colt_ast.h"

/// @brief If 'boolean' evaluates to true, returns ErrorExpr::CreateExpr(ctx)
#define IF_TRUE_RET_ERR(boolean) if ((boolean)) return ErrorExpr::CreateExpr(ctx)

namespace colt::lang
{
  Expected<AST, u32> CreateAST(StringView from, COLTContext& ctx) noexcept
  {
    AST result = { ctx };
    ASTMaker ast = { from, result.expressions, result.global_map, ctx };
    if (ast.is_empty() || ast.get_error_count() != 0)
      return { Error, ast.get_error_count() };
    else
      return { InPlace, std::move(result) };
  }

  u8 GetOpPrecedence(Token tkn) noexcept
  {
    static constexpr u8 operator_precedence_table[] =
    {
      10, 10, 11, 11, 11, // + - * / %
      6, 4, 5, 8, 8,  // & | ^ << >>
      3, 2,  // '&&' '||'
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

  bool isComparisonToken(Token tkn) noexcept
  {
    return TKN_GREAT_GREAT < tkn && tkn < TKN_EQUAL;
  }

  bool isTerminatedExpr(PTR<const Expr> expr) noexcept
  {
    switch (expr->classof())
    {
    case Expr::EXPR_SCOPE:
      return isTerminatedExpr(as<PTR<const ScopeExpr>>(expr)->get_body_array().get_back());
    case Expr::EXPR_ERROR:
    case Expr::EXPR_FN_RETURN:
      return true;
    case Expr::EXPR_CONDITION:
    {
      PTR<const ConditionExpr> cond = as<PTR<const ConditionExpr>>(expr);
      bool to_ret = true;
      //Validate both branches
      to_ret &= isTerminatedExpr(cond->get_if_statement());
      if (cond->get_else_statement() != nullptr)
        to_ret &= isTerminatedExpr(cond->get_else_statement());
      return to_ret;
    }
    default:
      return false;
    }
  }

  SourceCodeExprInfo ConcatInfo(const SourceCodeExprInfo& lhs, const SourceCodeExprInfo& rhs) noexcept
  {
    return SourceCodeExprInfo{ lhs.line_begin, rhs.line_end,
      StringView{lhs.lines.begin(), rhs.lines.end()},
      StringView{lhs.expression.begin(), rhs.expression.end()},
    };
  }

  SourceCodeExprInfo ASTMaker::SourceCodeLexemeInfo::to_src_info() const noexcept
  {
    return { as<u32>(line_nb), as<u32>(line_nb), line_strv, expression };
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

  ASTMaker::ASTMaker(StringView strv, Vector<PTR<Expr>>& expressions, Map<StringView, PTR<Expr>>& global_map, COLTContext& ctx) noexcept
    : expressions(expressions), lexer(strv), global_map(global_map), ctx(ctx)
  {
    current_tkn = lexer.get_next_token();
    while (current_tkn != TKN_EOF)
      expressions.push_back(parse_global_declaration());
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
      to_ret = LiteralExpr::CreateExpr(lexer.get_parsed_value(), BuiltInType::CreateBool(false, ctx),
        line_state.to_src_info(), ctx);
    break; case TKN_U8_L:
      consume_current_tkn();
      to_ret = LiteralExpr::CreateExpr(lexer.get_parsed_value(), BuiltInType::CreateU8(false, ctx),
        line_state.to_src_info(), ctx);
    break; case TKN_U16_L:
      consume_current_tkn();
      to_ret = LiteralExpr::CreateExpr(lexer.get_parsed_value(), BuiltInType::CreateU16(false, ctx),
        line_state.to_src_info(), ctx);
    break; case TKN_U32_L:
      consume_current_tkn();
      to_ret = LiteralExpr::CreateExpr(lexer.get_parsed_value(), BuiltInType::CreateU32(false, ctx),
        line_state.to_src_info(), ctx);
    break; case TKN_U64_L:
      consume_current_tkn();
      to_ret = LiteralExpr::CreateExpr(lexer.get_parsed_value(), BuiltInType::CreateU64(false, ctx),
        line_state.to_src_info(), ctx);
    break; case TKN_I8_L:
      consume_current_tkn();
      to_ret = LiteralExpr::CreateExpr(lexer.get_parsed_value(), BuiltInType::CreateI8(false, ctx),
        line_state.to_src_info(), ctx);
    break; case TKN_I16_L:
      consume_current_tkn();
      to_ret = LiteralExpr::CreateExpr(lexer.get_parsed_value(), BuiltInType::CreateI16(false, ctx),
        line_state.to_src_info(), ctx);
    break; case TKN_I32_L:
      consume_current_tkn();
      to_ret = LiteralExpr::CreateExpr(lexer.get_parsed_value(), BuiltInType::CreateI32(false, ctx),
        line_state.to_src_info(), ctx);
    break; case TKN_I64_L:
      consume_current_tkn();
      to_ret = LiteralExpr::CreateExpr(lexer.get_parsed_value(), BuiltInType::CreateI64(false, ctx),
        line_state.to_src_info(), ctx);
    break; case TKN_FLOAT_L:
      consume_current_tkn();
      to_ret = LiteralExpr::CreateExpr(lexer.get_parsed_value(), BuiltInType::CreateF32(false, ctx),
        line_state.to_src_info(), ctx);
    break; case TKN_DOUBLE_L:
      consume_current_tkn();
      to_ret = LiteralExpr::CreateExpr(lexer.get_parsed_value(), BuiltInType::CreateF64(false, ctx),
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
    case TKN_STAR:         // *(var)
    case TKN_TILDE:        // ~(any)
    case TKN_BANG:         // !(any) <- bool only
    case TKN_MINUS:        // -(any)
    case TKN_PLUS:         // +(any) <- error
      to_ret = parse_unary();

    break; case TKN_IDENTIFIER:
      to_ret = parse_identifier(line_state);

    break; case TKN_ERROR: //Lexer will have generated an error
      consume_current_tkn();
      ++error_count;
      to_ret = ErrorExpr::CreateExpr(ctx);

    break; case TKN_LEFT_PAREN:
      to_ret = parse_parenthesis(&ASTMaker::parse_binary, static_cast<u8>(0));

    break; default:
      generate_any_current<report_as::ERROR>(&ASTMaker::panic_consume_semicolon, "Expected an expression!");
      return ErrorExpr::CreateExpr(ctx);
    }
    return to_ret;
  }

  PTR<Expr> ASTMaker::parse_binary(u8 precedence) noexcept
  {
    if (precedence == 255) //token was not an operator: error
    {
      generate_any_current<report_as::ERROR>(&ASTMaker::panic_consume_semicolon, "Expected a binary operator!");
      return ErrorExpr::CreateExpr(ctx);
    }
    //Save current expression state
    SavedExprInfo line_state = { *this };


    PTR<Expr> lhs = parse_primary();
    //Save the current binary operators
    Token binary_op = current_tkn;
    //Source code informations of the operator
    auto binary_op_info = get_expr_info();

    //As assignment operators are right associative, they are handled
    //in a different function
    if (isAssignmentToken(binary_op))
      return parse_assignment(lhs, line_state);
    if (current_tkn == TKN_KEYWORD_AS) // EXPR as TYPE <- conversion
      return parse_conversion(lhs, line_state);

    //The current operator's precedence
    u8 op_precedence = GetOpPrecedence(binary_op);

    while (op_precedence > precedence)
    {
      if (op_precedence == 255) //token was not an operator: error
      {
        generate_any_current<report_as::ERROR>(&ASTMaker::panic_consume_semicolon, "Expected a ';'!");
        return ErrorExpr::CreateExpr(ctx);
      }

      //Consume the operator
      consume_current_tkn();
      //Recurse: 10 + 5 + 8 -> (10 + (5 + 8))
      PTR<Expr> rhs = parse_binary(GetOpPrecedence(binary_op));

      if (!rhs->get_type()->is_equal(lhs->get_type()))
      {
        generate_any<report_as::ERROR>(line_state.to_src_info(), &ASTMaker::panic_consume_semicolon,
          "Operands should be of same type!");
        return ErrorExpr::CreateExpr(ctx);
      }
      else if (is_a<BuiltInType>(rhs->get_type())
        && !as<PTR<const BuiltInType>>(rhs->get_type())->supports(TokenToBinaryOperator(binary_op)))
      {
        generate_any<report_as::ERROR>(line_state.to_src_info(), &ASTMaker::panic_consume_semicolon,
          "Type does not support operator '{}'!", binary_op_info.expression);
        return ErrorExpr::CreateExpr(ctx);
      }

      //Pratt's parsing, which allows operators priority
      lhs = BinaryExpr::CreateExpr(
        isComparisonToken(binary_op) ? BuiltInType::CreateBool(false, ctx) : lhs->get_type(),
        lhs, binary_op, rhs,
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

    if (op == TKN_PLUS)
    {
      generate_any<report_as::ERROR>(line_state.to_src_info(), &ASTMaker::panic_consume_semicolon,
        "Unary '+' is not supported!");
      return ErrorExpr::CreateExpr(ctx);
    }

    if (op == TKN_PLUS_PLUS || op == TKN_MINUS_MINUS)
    {
      PTR<Expr> read = parse_primary();
      if (!is_a<VarReadExpr>(read))
      {
        generate_any<report_as::ERROR>(line_state.to_src_info(), nullptr,
          "Increment/Decrement operator can only be applied on variables!");
        return ErrorExpr::CreateExpr(ctx);
      }
      //TODO: implement
      return ErrorExpr::CreateExpr(ctx);
    }

    //Dereference operator
    if (op == TKN_STAR)
    {
      auto expr = parse_primary();
      if (!is_a<VarReadExpr>(expr))
      {
        generate_any<report_as::ERROR>(line_state.to_src_info(), nullptr,
          "Dereference operator '*' can only be applied on variables!");
        return ErrorExpr::CreateExpr(ctx);
      }
      if (!expr->get_type()->is_ptr())
      {
        generate_any<report_as::ERROR>(line_state.to_src_info(), nullptr,
          "Dereference operator '*' can only be applied on pointer types!");
        return ErrorExpr::CreateExpr(ctx);
      }
      else
      {
        return UnaryExpr::CreateExpr(as<PTR<const PtrType>>(expr->get_type())->get_type_to(),
          TKN_STAR, false, expr, line_state.to_src_info(), ctx);
      }
    }

    //Parse the child expression -(5 + 8) -> PARENT -, CHILD (5 + 8)
    PTR<Expr> child = parse_primary();

    //No need to consume a Token as the previous call to parse_primary
    //already does
    return UnaryExpr::CreateExpr(child->get_type(), op, child,
      line_state.to_src_info(), ctx);
  }

  PTR<Expr> ASTMaker::parse_global_declaration() noexcept
  {
    if (current_tkn == TKN_KEYWORD_FN || current_tkn == TKN_KEYWORD_EXTERN)
    {
      auto expr = parse_fn_decl(); //Function
      if (is_a<FnDefExpr>(expr)) //add to the global table
        global_map.insert_or_assign(as<PTR<FnDefExpr>>(expr)->get_name(), expr);
      return expr;
    }
    else if (current_tkn == TKN_KEYWORD_VAR)
    {
      //Global Variable
      return parse_variable_decl(true);
    }
    else if (current_tkn == TKN_ERROR) //most likely invalid character
    {
      consume_current_tkn(); //consume TKN_ERROR
      ++error_count;
      return ErrorExpr::CreateExpr(ctx);
    }
    else
    {
      generate_any<report_as::ERROR>(get_expr_info().to_src_info(),
        &ASTMaker::panic_consume_decl, "Expected a declaration!");
      return ErrorExpr::CreateExpr(ctx);
    }
  }

  PTR<Expr> ASTMaker::parse_fn_decl() noexcept
  {
    SavedExprInfo line_state = { *this };

    assert(current_tkn == TKN_KEYWORD_FN || current_tkn == TKN_KEYWORD_EXTERN);

    bool is_extern = false;
    if (current_tkn == TKN_KEYWORD_EXTERN)
    {
      is_extern = true;
      consume_current_tkn();
    }

    consume_current_tkn();
    auto fn_name = lexer.get_parsed_identifier();

    IF_TRUE_RET_ERR(
      check_and_consume(TKN_IDENTIFIER, &ASTMaker::panic_consume_fn_decl,
        "Expected an identifier, not '{}'!", lexer.get_current_lexeme())
    );
    IF_TRUE_RET_ERR(
      check_and_consume(TKN_LEFT_PAREN, &ASTMaker::panic_consume_fn_decl,
        "Expected a '('!")
    );

    bool is_vararg = false;
    SmallVector<PTR<const Type>, 4> args_type;
    SmallVector<StringView, 4> args_name;
    while (current_tkn != TKN_EOF && current_tkn != TKN_RIGHT_PAREN)
    {
      SavedExprInfo line_state_arg = { *this };

      if (lexer.get_current_lexeme() == "va_arg")
      {
        is_vararg = true;
        consume_current_tkn(); //consume va_arg
        break;
      }

      args_type.push_back(parse_typename());
      auto arg_name = lexer.get_parsed_identifier();
      if (check_and_consume(TKN_IDENTIFIER, &ASTMaker::panic_consume_rparen, "Expected an identifier!"))
        break;

      if (args_name.to_view().contains(arg_name))
      {
        generate_any<report_as::ERROR>(line_state_arg.to_src_info(), &ASTMaker::panic_consume_rparen,
          "Cannot have parameters of same name '{}'!", arg_name);
        break;
      }
      args_name.push_back(arg_name);

      if (current_tkn == TKN_RIGHT_PAREN)
        break;
      if (check_and_consume(TKN_COMMA, &ASTMaker::panic_consume_rparen, "Expected a ','!"))
        break;
    }

    IF_TRUE_RET_ERR(
      check_and_consume(TKN_RIGHT_PAREN, &ASTMaker::panic_consume_fn_decl,
        "Expected a ')'!")
    );
    IF_TRUE_RET_ERR(
      check_and_consume(TKN_MINUS_GREAT, &ASTMaker::panic_consume_fn_decl,
        "Expected a '->'!")
    );

    //The return type of the function
    PTR<const Type> return_t = parse_typename();

    PTR<const Type> fn_ptr_t = FnType::CreateFn(return_t, std::move(args_type), is_vararg, ctx);
    PTR<FnDeclExpr> declaration = as<PTR<FnDeclExpr>>(FnDeclExpr::CreateExpr(fn_ptr_t, fn_name, std::move(args_name), is_extern, line_state.to_src_info(), ctx));    

    //Set the current function being parsed
    current_function = declaration;
    //And reset it on scope exit
    ON_EXIT{ current_function = nullptr; };

    //If 'main' function, check declaration
    if (declaration->get_name() == "main"
      && !is_cpp_equivalent<i64(*)(void)>(fn_ptr_t))
    {
      generate_any<report_as::ERROR>(declaration->get_src_code(), &ASTMaker::panic_consume_fn_decl,
        "Function 'main' should be declared as 'fn main()->i64'!");
      return ErrorExpr::CreateExpr(ctx);
    }

    if (is_valid_scope_begin() && !is_extern && !is_vararg)
    {
      SavedLocalState local_state = { *this };
      //Create arguments in local variables table
      for (size_t i = 0; i < declaration->get_params_count(); i++)
        local_var_table.push_back({ declaration->get_params_name()[i], declaration->get_params_type()[i] });

      auto body = parse_scope();
      if (!current_function->get_return_type()->is_void() && declaration->get_name() != "main")
        validate_all_path_return(body);
      //If a return is not present at the end of the void function,
      //add one. As parse_scope can return ErrorExpr or ScopeExpr,
      //do necessary check
      else if (is_a<ScopeExpr>(body) && !isTerminatedExpr(body))
      {
        //If main has no return, add 'return 0'
        //If function is not main, (and returns void) add 'return void'
        as<PTR<ScopeExpr>>(body)->push_back(FnReturnExpr::CreateExpr(
          declaration->get_name() != "main" ? nullptr :
          LiteralExpr::CreateValue(0LL, ctx), {}, ctx)
        );
      }

      return FnDefExpr::CreateExpr(declaration, body, line_state.to_src_info(), ctx);
    }
    if (!is_extern && is_vararg)
    {
      generate_any<report_as::ERROR>(declaration->get_src_code(), &ASTMaker::panic_consume_fn_decl,
        "Function using C-style variadic can only be extern!");
      return ErrorExpr::CreateExpr(ctx);
    }
    check_and_consume(TKN_SEMICOLON, "Expected a ';'!");
    return FnDefExpr::CreateExpr(declaration, line_state.to_src_info(), ctx);
  }

  PTR<Expr> ASTMaker::parse_scope(bool one_expr) noexcept
  {
    SavedExprInfo line_state = { *this };
    if (current_tkn == TKN_COLON && one_expr)
    {
      consume_current_tkn(); // :

      Vector<PTR<Expr>> statements = {};
      statements.push_back(parse_statement());
      //We still want to return a ScopeExpr even for a single expression
      return ScopeExpr::CreateExpr(std::move(statements),
        line_state.to_src_info(), ctx);
    }
    else if (current_tkn == TKN_LEFT_CURLY)
    {
      //Save '{' informations
      auto lexeme_info = get_expr_info();
      consume_current_tkn(); // '{'

      Vector<PTR<Expr>> statements = {};
      while (current_tkn != TKN_RIGHT_CURLY && current_tkn != TKN_EOF)
      {
        auto stt = parse_statement();
        statements.push_back(stt);

        if ((is_a<BreakContinueExpr>(stt) || is_a<FnReturnExpr>(stt))
          && current_tkn != TKN_RIGHT_CURLY)
          handle_unreachable_code();
      }

      if (current_tkn != TKN_RIGHT_CURLY)
        generate_any<report_as::ERROR>(lexeme_info.to_src_info(), nullptr,
          "Unclosed curly bracket delimiter!");
      else //consume '}'
        consume_current_tkn();

      //If empty scope, push a no-op
      if (statements.is_empty())
        statements.push_back(NoOpExpr::CreateExpr(line_state.to_src_info(), ctx));

      return ScopeExpr::CreateExpr(std::move(statements),
        line_state.to_src_info(), ctx);
    }
    else
    {
      //TODO: choose consuming strategy
      generate_any_current<report_as::ERROR>(nullptr, "Expected the beginning of a scope ('{{'{}", one_expr ? "or ':')!" : ")!");
      return ErrorExpr::CreateExpr(ctx);
    }
  }

  PTR<Expr> ASTMaker::parse_statement() noexcept
  {
    assert_true(current_function, "Parse statement can only happen inside a function!");

    SavedExprInfo line_state = { *this };

    bool is_valid = true; //modified by continue/break handling
    PTR<Expr> to_ret;
    switch (current_tkn)
    {
    case TKN_KEYWORD_VAR:
      return parse_variable_decl(false);
    case TKN_LEFT_CURLY:
      return parse_scope(false);
    case TKN_KEYWORD_IF:
      return parse_condition();
    case TKN_KEYWORD_WHILE:
      return parse_while();
    break; case TKN_KEYWORD_RETURN:
      return parse_return();

    case TKN_SEMICOLON:
      generate_any_current<report_as::ERROR>(nullptr, "Expected a statement!");
      consume_current_tkn(); // ';'
      return ErrorExpr::CreateExpr(ctx);

      // EXPECTS ';'

    case TKN_KEYWORD_CONTINUE:
      to_ret = BreakContinueExpr::CreateExpr(false, get_expr_info().to_src_info(), ctx);
      consume_current_tkn();
      if (!is_parsing_loop)
      {
        generate_any<report_as::ERROR>(to_ret->get_src_code(), nullptr, "Statement 'continue' can only appear inside a loop!");
        is_valid = false;
      }

    break; case TKN_KEYWORD_BREAK:
      to_ret = BreakContinueExpr::CreateExpr(true, get_expr_info().to_src_info(), ctx);
      consume_current_tkn();
      if (!is_parsing_loop)
      {
        generate_any<report_as::ERROR>(to_ret->get_src_code(), nullptr, "Statement 'break' can only appear inside a loop!");
        is_valid = false;
      }

    break; default:
      if (lexer.get_current_lexeme() == "pass")
      {
        consume_current_tkn();
        to_ret = NoOpExpr::CreateExpr(line_state.to_src_info(), ctx);
      }
      else
        to_ret = parse_binary();
    }
    //TODO: recheck strategy
    check_and_consume(TKN_SEMICOLON, &ASTMaker::panic_consume_sttmnt,
      "Expected a ';'!");
    if (is_valid)
      return to_ret;
    return ErrorExpr::CreateExpr(ctx);
  }

  PTR<Expr> ASTMaker::parse_condition() noexcept
  {
    assert(current_tkn == TKN_KEYWORD_IF);
    SavedExprInfo line_state = { *this };

    consume_current_tkn(); //consume if

    PTR<Expr> if_cond = parse_binary(); //if condition
    if (!if_cond->get_type()->is_equal(BuiltInType::CreateBool(false, ctx)))
      generate_any<report_as::ERROR>(if_cond->get_src_code(), nullptr,
        "Expression should be of type 'bool'!");
    //If the expression is not a comparison, but is of type bool
    //(read from boolean variable, ...), transform it
    //into a comparison with 'true'
    else if (!is_a<BinaryExpr>(if_cond))
      if_cond = BinaryExpr::CreateExpr(if_cond->get_type(), if_cond, TKN_EQUAL_EQUAL,
        LiteralExpr::CreateValue(true, ctx), if_cond->get_src_code(), ctx);

    PTR<Expr> if_body = parse_scope(); //if body

    if (current_tkn == TKN_KEYWORD_ELIF)
    {
      current_tkn = TKN_KEYWORD_IF;
      PTR<Expr> else_body = parse_condition(); //we recurse
      return ConditionExpr::CreateExpr(if_cond, if_body, else_body,
        line_state.to_src_info(), ctx);
    }
    PTR<Expr> else_body = nullptr;
    if (current_tkn == TKN_KEYWORD_ELSE)
    {
      consume_current_tkn(); //consume else
      else_body = parse_scope(); // else body
    }
    return ConditionExpr::CreateExpr(if_cond, if_body, else_body,
      line_state.to_src_info(), ctx);
  }

  PTR<Expr> ASTMaker::parse_while() noexcept
  {
    assert(current_tkn == TKN_KEYWORD_WHILE);
    //Save loop state
    bool old_is_loop = is_parsing_loop;
    is_parsing_loop = true;

    SavedExprInfo line_state = { *this };

    consume_current_tkn(); //consume while

    PTR<Expr> condition = parse_binary();
    if (!condition->get_type()->is_equal(BuiltInType::CreateBool(false, ctx)))
      generate_any<report_as::ERROR>(condition->get_src_code(), nullptr,
        "Expression should be of type 'bool'!");
    //If the expression is not a comparison, but is of type bool
    //(read from boolean variable, ...), transform it
    //into a comparison with 'true'
    else if (!is_a<BinaryExpr>(condition))
      condition = BinaryExpr::CreateExpr(condition->get_type(), condition, TKN_EQUAL_EQUAL,
        LiteralExpr::CreateValue(true, ctx), condition->get_src_code(), ctx);

    PTR<Expr> body = parse_scope();

    //Restore loop state
    is_parsing_loop = old_is_loop;

    return WhileLoopExpr::CreateExpr(condition, body,
      line_state.to_src_info(), ctx);
  }

  PTR<Expr> ASTMaker::parse_variable_decl(bool is_global) noexcept
  {
    SavedExprInfo line_state = { *this };

    IF_TRUE_RET_ERR(
      check_and_consume(TKN_KEYWORD_VAR, &ASTMaker::panic_consume_var_decl, "Expected a variable declaration!")
    );
    IF_TRUE_RET_ERR(
      check_and_consume(TKN_IDENTIFIER, &ASTMaker::panic_consume_var_decl, "Expected an identifier!")
    );

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
      IF_TRUE_RET_ERR(
        check_and_consume(TKN_EQUAL, &ASTMaker::panic_consume_var_decl, "Expected a '='!")
      );
      var_init = parse_binary();
    }
    else if (var_type == nullptr)
    {
      generate_any<report_as::ERROR>(line_state.to_src_info(), &ASTMaker::panic_consume_var_decl,
        "An uninitialized variable should specify its type!");
      return ErrorExpr::CreateExpr(ctx);
    }
    else //uninitialized variable with explicit type
    {
      consume_current_tkn();
      goto GEN; //skip type conversions
    }

    //If the type is not explicit, deduce it from left hand side
    //else convert left hand side to explicit type
    if (var_type == nullptr)
      var_type = var_init->get_type();
    else
      var_init = ConvertExpr::CreateExpr(var_type, var_init,
        line_state.to_src_info(), ctx);

    IF_TRUE_RET_ERR(
      check_and_consume(TKN_SEMICOLON, &ASTMaker::panic_consume_var_decl, "Expected a ';'!")
    );

  GEN:
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

  PTR<Expr> ASTMaker::parse_assignment(PTR<Expr> lhs, const SavedExprInfo& line_state) noexcept
  {
    Token assignment_tkn = current_tkn;
    consume_current_tkn(); //consume '='
    PTR<Expr> rhs = parse_binary();

    if (!is_a<VarReadExpr>(lhs))
    {
      generate_any<report_as::ERROR>(lhs->get_src_code(), nullptr,
        "Left hand side of an assignment should be a variable!");
      return ErrorExpr::CreateExpr(ctx);
    }

    if (assignment_tkn == TKN_EQUAL)
      return VarWriteExpr::CreateExpr(as<PTR<VarReadExpr>>(lhs), rhs, line_state.to_src_info(), ctx);

    //Expands VAR += VALUE as VAR = VAR + VALUE
    switch (assignment_tkn)
    {
    case colt::lang::TKN_PLUS_EQUAL:
      return VarWriteExpr::CreateExpr(as<PTR<VarReadExpr>>(lhs),
        BinaryExpr::CreateExpr(lhs->get_type(), lhs, TKN_PLUS, rhs, line_state.to_src_info(), ctx),
        line_state.to_src_info(), ctx);
    case colt::lang::TKN_MINUS_EQUAL:
      return VarWriteExpr::CreateExpr(as<PTR<VarReadExpr>>(lhs),
        BinaryExpr::CreateExpr(lhs->get_type(), lhs, TKN_MINUS, rhs, line_state.to_src_info(), ctx),
        line_state.to_src_info(), ctx);
    case colt::lang::TKN_STAR_EQUAL:
      return VarWriteExpr::CreateExpr(as<PTR<VarReadExpr>>(lhs),
        BinaryExpr::CreateExpr(lhs->get_type(), lhs, TKN_STAR, rhs, line_state.to_src_info(), ctx),
        line_state.to_src_info(), ctx);
    case colt::lang::TKN_SLASH_EQUAL:
      return VarWriteExpr::CreateExpr(as<PTR<VarReadExpr>>(lhs),
        BinaryExpr::CreateExpr(lhs->get_type(), lhs, TKN_SLASH, rhs, line_state.to_src_info(), ctx),
        line_state.to_src_info(), ctx);
    case colt::lang::TKN_PERCENT_EQUAL:
      return VarWriteExpr::CreateExpr(as<PTR<VarReadExpr>>(lhs),
        BinaryExpr::CreateExpr(lhs->get_type(), lhs, TKN_PERCENT, rhs, line_state.to_src_info(), ctx),
        line_state.to_src_info(), ctx);
    case colt::lang::TKN_AND_EQUAL:
      return VarWriteExpr::CreateExpr(as<PTR<VarReadExpr>>(lhs),
        BinaryExpr::CreateExpr(lhs->get_type(), lhs, TKN_AND, rhs, line_state.to_src_info(), ctx),
        line_state.to_src_info(), ctx);
    case colt::lang::TKN_OR_EQUAL:
      return VarWriteExpr::CreateExpr(as<PTR<VarReadExpr>>(lhs),
        BinaryExpr::CreateExpr(lhs->get_type(), lhs, TKN_OR, rhs, line_state.to_src_info(), ctx),
        line_state.to_src_info(), ctx);
    case colt::lang::TKN_CARET_EQUAL:
      return VarWriteExpr::CreateExpr(as<PTR<VarReadExpr>>(lhs),
        BinaryExpr::CreateExpr(lhs->get_type(), lhs, TKN_CARET, rhs, line_state.to_src_info(), ctx),
        line_state.to_src_info(), ctx);
    case colt::lang::TKN_LESS_LESS_EQUAL:
      return VarWriteExpr::CreateExpr(as<PTR<VarReadExpr>>(lhs),
        BinaryExpr::CreateExpr(lhs->get_type(), lhs, TKN_LESS_LESS, rhs, line_state.to_src_info(), ctx),
        line_state.to_src_info(), ctx);
    case colt::lang::TKN_GREAT_GREAT_EQUAL:
      return VarWriteExpr::CreateExpr(as<PTR<VarReadExpr>>(lhs),
        BinaryExpr::CreateExpr(lhs->get_type(), lhs, TKN_GREAT_GREAT, rhs, line_state.to_src_info(), ctx),
        line_state.to_src_info(), ctx);
    default:
      colt_unreachable("Invalid assignment token!");
    }
  }

  PTR<Expr> ASTMaker::parse_conversion(PTR<Expr> lhs, const SavedExprInfo& line_state) noexcept
  {
    assert(current_tkn == TKN_KEYWORD_AS);

    consume_current_tkn();
    PTR<const Type> cnv_type = parse_typename();

    return ConvertExpr::CreateExpr(cnv_type, lhs,
      line_state.to_src_info(), ctx);
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

    bool is_const = true;
    if (current_tkn == TKN_KEYWORD_MUT) // mut TYPE
    {
      is_const = false;
      consume_current_tkn();
    }

    switch (current_tkn)
    {
    case TKN_KEYWORD_VOID:
    {
      if (!is_const)
        generate_any<report_as::ERROR>(line_state.to_src_info(), nullptr,
          "'void' typename cannot be marked as mutable!");
      consume_current_tkn(); //void
      return VoidType::CreateType(ctx);
    }
    case TKN_KEYWORD_BOOL:
      consume_current_tkn();
      return BuiltInType::CreateBool(is_const, ctx);
    case TKN_KEYWORD_CHAR:
      //TODO: add
      colt_unreachable("not implemented");
      //return BuiltInType::CreateChar(is_const, ctx);
    case TKN_KEYWORD_I8:
      consume_current_tkn();
      return BuiltInType::CreateI8(is_const, ctx);
    case TKN_KEYWORD_U8:
      consume_current_tkn();
      return BuiltInType::CreateU8(is_const, ctx);
    case TKN_KEYWORD_I16:
      consume_current_tkn();
      return BuiltInType::CreateI16(is_const, ctx);
    case TKN_KEYWORD_U16:
      consume_current_tkn();
      return BuiltInType::CreateU16(is_const, ctx);
    case TKN_KEYWORD_I32:
      consume_current_tkn();
      return BuiltInType::CreateI32(is_const, ctx);
    case TKN_KEYWORD_U32:
      consume_current_tkn();
      return BuiltInType::CreateU32(is_const, ctx);
    case TKN_KEYWORD_I64:
      consume_current_tkn();
      return BuiltInType::CreateI64(is_const, ctx);
    case TKN_KEYWORD_U64:
      consume_current_tkn();
      return BuiltInType::CreateU64(is_const, ctx);
    case TKN_KEYWORD_FLOAT:
      consume_current_tkn();
      return BuiltInType::CreateF32(is_const, ctx);
    case TKN_KEYWORD_DOUBLE:
      consume_current_tkn();
      return BuiltInType::CreateF64(is_const, ctx);
    case TKN_KEYWORD_LSTRING:
      //TODO: add
      colt_unreachable("not implemented");
    case TKN_KEYWORD_PTR:
    {
      consume_current_tkn();
      //TODO: pass strategy to consume
      if (!check_and_consume(TKN_LESS, "Expected a '<'!"))
      {
        PTR<const Type> ptr_to = parse_typename();
        if (current_tkn == TKN_GREAT_GREAT) // '>>' is parsed as '>' '>'
          current_tkn = TKN_GREAT;
        if (!check_and_consume(TKN_GREAT, "Expected a '>'!"))
          return PtrType::CreatePtr(is_const, ptr_to, ctx);
      }
    }
    break;
    case TKN_IDENTIFIER:
      //TODO: add
      colt_unreachable("not implemented");
    default:
      generate_any<report_as::ERROR>(line_state.to_src_info(), nullptr,
        "Expected a typename!");
    }
    //return an error
    return ErrorType::CreateType(ctx);
  }

  PTR<Expr> ASTMaker::parse_identifier(const SavedExprInfo& line_state) noexcept
  {
    assert(current_tkn == TKN_IDENTIFIER);

    StringView identifier = lexer.get_parsed_identifier();
    consume_current_tkn(); // consume identifier
    //The source code information of the identifier, done AFTER consuming
    SourceCodeExprInfo identifier_info = line_state.to_src_info();

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
    }
    if (auto gvar = global_map.find(identifier); gvar != nullptr)
    {
      if (!is_a<VarDeclExpr>(gvar->second))
      {
        generate_any<report_as::ERROR>(identifier_info, nullptr,
          "'{}' is not a variable!", identifier);
        return ErrorExpr::CreateExpr(ctx);
      }
      return VarReadExpr::CreateExpr(gvar->second->get_type(), identifier,
        line_state.to_src_info(), ctx);
    }
    else
    {
      generate_any<report_as::ERROR>(identifier_info, nullptr,
        "Variable of name '{}' does not exist!", identifier);
      return ErrorExpr::CreateExpr(ctx);
    }
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
        if (!validate_fn_call(arguments, decl, identifier, identifier_location))
          return ErrorExpr::CreateExpr(ctx);
        return FnCallExpr::CreateExpr(decl, std::move(arguments),
          line_state.to_src_info(), ctx);
      }
      else // and return an ErrorExpr
        generate_any<report_as::ERROR>(identifier_location, nullptr,
          "'{}' is not a function!", identifier);
    }
    else
      generate_any<report_as::ERROR>(identifier_location, nullptr,
        "Function of name '{}' does not exist!", identifier);
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

  PTR<Expr> ASTMaker::parse_return() noexcept
  {
    assert(current_tkn == TKN_KEYWORD_RETURN);

    SavedExprInfo line_state = { *this };
    consume_current_tkn();

    if (current_function->get_return_type()->is_void())
    {
      if (current_tkn != TKN_SEMICOLON)
      {
        //Save state after 'return' keyword
        SavedExprInfo cnsm = { *this };

        //This will consume the last ';'
        ON_EXIT{ if (current_tkn == TKN_SEMICOLON) consume_current_tkn(); };
        //We consumed before calling 'to_src_info' to obtain the correct highlighting
        panic_consume_return();
        generate_any<report_as::ERROR>(cnsm.to_src_info(), nullptr,
          "Function '{}' of return type 'void' cannot return a value!", current_function->get_name());
        return ErrorExpr::CreateExpr(ctx);
      }
      consume_current_tkn(); // consume ';'
      return FnReturnExpr::CreateExpr(nullptr, line_state.to_src_info(), ctx);
    }
    PTR<Expr> ret_val = parse_binary();
    if (!ret_val->get_type()->is_equal(current_function->get_return_type()))
    {
      generate_any<report_as::ERROR>(ret_val->get_src_code(), nullptr,
        "Type of return value does not match function return type!");
      ret_val = ErrorExpr::CreateExpr(ctx);
    }
    else
      ret_val = FnReturnExpr::CreateExpr(ret_val, line_state.to_src_info(), ctx);

    check_and_consume(TKN_SEMICOLON, &ASTMaker::panic_consume_sttmnt,
      "Expected a ';'!");
    return ret_val;
  }

  bool ASTMaker::validate_fn_call(const SmallVector<PTR<Expr>, 4>& arguments, PTR<const FnDeclExpr> decl, StringView identifier, const SourceCodeExprInfo& info) noexcept
  {
    if (arguments.get_size() != decl->get_params_count())
    {
      if (as<PTR<const FnType>>(decl->get_type())->is_varargs())
      {
        if (arguments.get_size() < decl->get_params_count())
        {
          generate_any<report_as::ERROR>(info, nullptr, "Variadic function '{}' expects at least {} argument{} not {}!", identifier,
            decl->get_params_count(), decl->get_params_count() == 1 ? "," : "s,", arguments.get_size());
          return false;
        }
      }
      else
      {
        generate_any<report_as::ERROR>(info, nullptr, "Function '{}' expects {} argument{} not {}!", identifier,
          decl->get_params_count(), decl->get_params_count() == 1 ? "," : "s,", arguments.get_size());
        return false;
      }
    }
    bool ret = true;
    for (size_t i = 0; i < decl->get_params_count(); i++)
    {
      if (!arguments[i]->get_type()->is_equal(decl->get_params_type()[i]))
      {
        //TODO: checking for pointer types
        generate_any<report_as::ERROR>(arguments[i]->get_src_code(), nullptr,
          "Type of argument does not match that of declaration!");
        ret = false;
      }
    }
    return ret;
  }

  void ASTMaker::handle_unreachable_code() noexcept
  {
    PTR<const Expr> stt = parse_statement();
    SourceCodeExprInfo stt_info = stt->get_src_code();
    while (current_tkn != TKN_RIGHT_CURLY && current_tkn != TKN_EOF)
      stt = parse_statement();

    generate_any<report_as::WARNING>(ConcatInfo(stt_info, stt->get_src_code()),
      nullptr, "Unreachable code!");
  }

  void ASTMaker::validate_all_path_return(PTR<const Expr> expr) noexcept
  {
    switch (expr->classof())
    {
    case Expr::EXPR_SCOPE:
      validate_all_path_return(as<PTR<const ScopeExpr>>(expr)->get_body_array().get_back());
      [[fallthrough]];
    case Expr::EXPR_ERROR:
    case Expr::EXPR_FN_RETURN:
      return;
    case Expr::EXPR_CONDITION:
    {
      PTR<const ConditionExpr> cond = as<PTR<const ConditionExpr>>(expr);
      //Validate both branches
      validate_all_path_return(cond->get_if_statement());
      if (cond->get_else_statement() != nullptr)
        validate_all_path_return(cond->get_else_statement());
      return;
    }
    default:
      generate_any<report_as::ERROR>(expr->get_src_code(), nullptr,
        "Expected a 'return' statement, as path must return a value!");
    }
  }

  void ASTMaker::panic_consume_semicolon() noexcept
  {
    while (current_tkn != TKN_SEMICOLON && current_tkn != TKN_RIGHT_CURLY && current_tkn != TKN_EOF)
      consume_current_tkn();
  }

  void ASTMaker::panic_consume_decl() noexcept
  {
    while (current_tkn != TKN_KEYWORD_VAR && current_tkn != TKN_KEYWORD_FN && current_tkn != TKN_EOF)
      consume_current_tkn();
  }

  void ASTMaker::panic_consume_return() noexcept
  {
    while (current_tkn != TKN_SEMICOLON && current_tkn != TKN_RIGHT_CURLY && current_tkn != TKN_EOF
      && current_tkn != TKN_KEYWORD_IF && current_tkn != TKN_KEYWORD_WHILE && current_tkn != TKN_KEYWORD_VAR)
      consume_current_tkn();
  }

  void ASTMaker::panic_consume_sttmnt() noexcept
  {
    while (current_tkn != TKN_SEMICOLON && current_tkn != TKN_RIGHT_CURLY && current_tkn != TKN_EOF
      && current_tkn != TKN_KEYWORD_IF && current_tkn != TKN_KEYWORD_WHILE && current_tkn != TKN_KEYWORD_VAR)
      consume_current_tkn();
    if (current_tkn == TKN_SEMICOLON)
      consume_current_tkn();
  }

  void ASTMaker::panic_consume_var_decl() noexcept
  {
    while (current_tkn != TKN_SEMICOLON && current_tkn != TKN_RIGHT_CURLY && current_tkn != TKN_EOF)
      consume_current_tkn();
    if (current_tkn == TKN_SEMICOLON)
      consume_current_tkn();
  }

  void ASTMaker::panic_consume_fn_decl() noexcept
  {
    while (current_tkn != TKN_KEYWORD_FN && current_tkn != TKN_KEYWORD_VAR && current_tkn != TKN_EOF)
      consume_current_tkn();
  }

  void ASTMaker::panic_consume_rparen() noexcept
  {
    while (current_tkn != TKN_SEMICOLON && current_tkn != TKN_RIGHT_PAREN && current_tkn != TKN_EOF)
      consume_current_tkn();
  }
}

#undef IF_TRUE_RET_ERR