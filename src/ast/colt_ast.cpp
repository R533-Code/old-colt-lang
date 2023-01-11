/** @file colt_ast.cpp
* Contains definition of functions declared in 'colt_ast.h'.
*/

#include "colt_ast.h"

namespace colt::lang
{
  Expected<AST, u32> CreateAST(StringView from, COLTContext& ctx) noexcept
  {
    AST result = { ctx };
    ASTMaker ast = { from, result };
    if (ast.is_empty() || ast.get_error_count() != 0)
      return { Error, ast.get_error_count() };
    else
      return { InPlace, std::move(result) };
  }

  bool CompileAndAdd(StringView str, AST& ast) noexcept
  {
    u64 crr = ast.expressions.get_size();
    if (ASTMaker astm = { str, ast };
      astm.get_error_count() != 0)
    {
      ast.expressions.pop_back_n(ast.expressions.get_size() - crr);
      return false;
    }
    return true;
  }

  u8 getOpPrecedence(Token tkn) noexcept
  {
    static constexpr u8 operator_precedence_table[] =
    {
      10, 10, 11, 11, 11, // + - * / %
      6, 4, 5, 8, 8,  // & | ^ << >>
      3, 2,  // '&&' '||'
      9, 9, 9, 9, 10, 10, // < <= > >= != ==
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // = += -= *= /= %= &= |= ^= <<= >>=
    };
    assert_true(static_cast<size_t>(tkn) >= 0, "Token should be greater or equal to 0!");
    if (tkn < TKN_COMMA)
      return operator_precedence_table[tkn];
    return 0;
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

  ASTMaker::ASTMaker(StringView strv, AST& ast) noexcept
    : expressions(ast.expressions), lexer(strv), global_map(ast.global_map), str_table(ast.str_table), ctx(ast.ctx)
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

  PTR<Expr> ASTMaker::parse_primary(bool cnv) noexcept
  {
    //Save current expression state
    SavedExprInfo line_state = { *this };

    PTR<Expr> to_ret;

    if (isLiteralToken(current_tkn))
    {
      //Literal tokens: 10.0, "Hello World"...
      //Those are transformed into an untyped QWORD and
      //returned as a LiteralExpr.

      //Save literal token
      auto literal_tkn = current_tkn;
      //Save the literal as a QWORD
      QWORD value;

      //If the token is a string literal, we save it to
      //the global string table.
      if (current_tkn == TKN_STRING_L)
        value = str_table.insert(lexer.get_string_literal()).first;
      else
        value = lexer.get_parsed_value();

      //Consume the literal token
      consume_current_tkn();

      to_ret = LiteralExpr::CreateExpr(value, literal_tkn,
        line_state.to_src_info(), ctx);
    }
    else if (current_tkn == TKN_IDENTIFIER)
      to_ret = parse_identifier(line_state);
    else if (isUnaryToken(current_tkn))
      to_ret = parse_unary();
    else if (current_tkn == TKN_LEFT_PAREN)
      to_ret = parse_parenthesis(&ASTMaker::parse_binary, static_cast<u8>(0));
    else if (current_tkn == TKN_ERROR)
    {
      //TKN_ERROR is an invalid lexeme (usually literal)
      //generated by the lexer. The lexer will have printed
      //the error, now just register this as being an error.
      consume_current_tkn();
      ++error_count; //register as error
      to_ret = ErrorExpr::CreateExpr(ctx);
    }
    else
    {
      //Invalid primary expression.
      generate_any_current<report_as::ERROR>(&ASTMaker::panic_consume_semicolon,
        "Expected an expression!");
      to_ret = ErrorExpr::CreateExpr(ctx);
    }

    if (cnv && (current_tkn == TKN_KEYWORD_AS
      || current_tkn == TKN_KEYWORD_BIT_AS)) // EXPR as TYPE <- conversion
      return parse_conversion(to_ret, line_state);
    return to_ret;
  }

  PTR<Expr> ASTMaker::parse_binary(u8 precedence) noexcept
  {
    //Save current expression state
    SavedExprInfo line_state = { *this };


    PTR<Expr> lhs = parse_primary();
    //Save the current binary operators
    Token binary_op = current_tkn;

    //As assignment operators are right associative, they are handled
    //in a different function
    if (isAssignmentToken(binary_op))
      return parse_assignment(lhs, line_state);

    //The current operator's precedence
    u8 op_precedence = getOpPrecedence(binary_op);

    while (op_precedence > precedence)
    {
      //Consume the operator
      consume_current_tkn();
      //Recurse: 10 + 5 + 8 -> (10 + (5 + 8))
      PTR<Expr> rhs = parse_binary(getOpPrecedence(binary_op));      

      //Pratt's parsing, which allows operators priority
      lhs = create_binary(
        isComparisonToken(binary_op) ? BuiltInType::CreateBool(false, ctx) : lhs->get_type(),
        lhs, binary_op, rhs,
        line_state.to_src_info());

      //Update the Token
      binary_op = current_tkn;
      //Update precedence
      op_precedence = getOpPrecedence(binary_op);
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

    switch (op)
    {
    case TKN_PLUS:
    {
      generate_any<report_as::ERROR>(line_state.to_src_info(), &ASTMaker::panic_consume_semicolon,
        "Unary '+' is not supported!");
      return ErrorExpr::CreateExpr(ctx);
    }
    case TKN_MINUS:
    {
      //Parse the child expression -(5 + 8) -> PARENT -, CHILD (5 + 8)
      PTR<Expr> child = parse_primary(false);
      if (child->get_type()->is_builtin()
        && !as<PTR<const BuiltInType>>(child->get_type())->is_signed())
      {
        generate_any<report_as::ERROR>(line_state.to_src_info(), nullptr,
          "Only signed integers and floating point types support negation operator '-'!");
        return ErrorExpr::CreateExpr(ctx);
      }
      //No need to consume a Token as the previous call to parse_primary
      //already does
      return UnaryExpr::CreateExpr(child->get_type(), op, child,
        line_state.to_src_info(), ctx);
    }
    
      // ++ -- operators. They only exist as prefix.
    case TKN_PLUS_PLUS:
    case TKN_MINUS_MINUS:
    {
      if (PTR<Expr> read = parse_primary(false);
        !is_a<VarReadExpr>(read))
      {
        generate_any<report_as::ERROR>(line_state.to_src_info(), nullptr,
          "'++' and '--' operator can only be applied on variables!");
      }
      else if (read->get_type()->is_floating()
        && read->get_type()->is_semantically_integral())
      {
        generate_any<report_as::ERROR>(line_state.to_src_info(), nullptr,
          "'++' and '--' operator can only be applied on floating points and integrals types!");
      }
      else if (read->get_type()->is_const())
      {
        generate_any<report_as::ERROR>(line_state.to_src_info(), nullptr,
          "'++' and '--' operator can only be applied on mutable variables!");
      }
      else //no error
      {
        QWORD value = {};
        if (read->get_type()->is_integral())
          value = 1;
        else if (read->get_type()->is_f32())
          value = 1.0f;
        else if (read->get_type()->is_f64())
          value = 1.0;
        //Generate an increment/decrement by reading the variable,
        //adding 1, then writing.
        return VarWriteExpr::CreateExpr(as<PTR<VarReadExpr>>(read),
          BinaryExpr::CreateExpr(read->get_type(), read,
          op == TKN_PLUS_PLUS ? TKN_PLUS : TKN_MINUS,
          LiteralExpr::CreateExpr(value, read->get_type(), line_state.to_src_info(), ctx),
            line_state.to_src_info(), ctx),
          line_state.to_src_info(), ctx);
      }
      return ErrorExpr::CreateExpr(ctx);
    }
      //Dereference operator '*': applied on pointers
    case TKN_STAR:
    {
      auto expr = parse_primary(false);
      if (!is_a<VarReadExpr>(expr))
      {
        generate_any<report_as::ERROR>(line_state.to_src_info(), nullptr,
          "Dereference operator '*' can only be applied on variables!");
        return ErrorExpr::CreateExpr(ctx);
      }
      if (!expr->get_type()->is_ptr()
        && expr->get_type()->is_ptr_to_void())
      {
        generate_any<report_as::ERROR>(line_state.to_src_info(), nullptr,
          "Dereference operator '*' can only be applied on non-void pointer types!");
        return ErrorExpr::CreateExpr(ctx);
      }
      //Load from pointer
      return PtrLoadExpr::CreateExpr(expr, line_state.to_src_info(), ctx);
    }

      //Address of operator '&': applied on variables
    case TKN_AND:
    {
      auto expr = parse_primary(false);
      if (!is_a<VarReadExpr>(expr))
      {
        generate_any<report_as::ERROR>(line_state.to_src_info(), nullptr,
          "Address of operator '&' can only be applied on variables!");
        return ErrorExpr::CreateExpr(ctx);
      }
      return UnaryExpr::CreateExpr(PtrType::CreatePtr(false, expr->get_type(), ctx),
        TKN_AND, expr, line_state.to_src_info(), ctx);
    }

    case TKN_TILDE:
    {
      auto expr = parse_primary(false);
      //pure integral: uint or int (without bool/char)
      if (expr->get_type()->is_builtin()
        && !as<PTR<const BuiltInType>>(expr->get_type())->is_semantically_integral())
      {
        generate_any<report_as::ERROR>(line_state.to_src_info(), nullptr,
         "Bit NOT '~' can only be applied on integral types!");
        return ErrorExpr::CreateExpr(ctx);
      }
      return UnaryExpr::CreateExpr(expr->get_type(),
        TKN_TILDE, expr, line_state.to_src_info(), ctx);
    }

    case TKN_BANG:
    {
      auto expr = parse_primary(false);
      //Can only be applied on booleans
      if (expr->get_type()->is_builtin()
        && !as<PTR<const BuiltInType>>(expr->get_type())->is_bool())
      {
        generate_any<report_as::ERROR>(line_state.to_src_info(), nullptr,
          "Bool NOT '!' can only be applied on 'bool' type!");
        return ErrorExpr::CreateExpr(ctx);
      }
      return UnaryExpr::CreateExpr(expr->get_type(),
        TKN_BANG, expr, line_state.to_src_info(), ctx);
    }
    default:
      colt_unreachable("Invalid unary token!");
    }
  }

  PTR<Expr> ASTMaker::parse_global_declaration() noexcept
  {
    //Function declaration/definition
    if (current_tkn == TKN_KEYWORD_FN || current_tkn == TKN_KEYWORD_EXTERN)
    {
      auto expr = parse_fn_decl(); //Function
      if (is_a<FnDefExpr>(expr)) //add to the global table
        add_fn_to_global_table(as<PTR<FnDefExpr>>(expr));
      return expr;
    }
    //Global variable declaration
    else if (current_tkn == TKN_KEYWORD_VAR)
    {
      //Global Variable
      return parse_var_decl(true);
    }
    //Most likely an invalid character
    else if (current_tkn == TKN_ERROR)
    {
      consume_current_tkn(); //consume TKN_ERROR
      ++error_count; //register the error
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

    if (check_and_consume(TKN_IDENTIFIER, &ASTMaker::panic_consume_fn_decl,
      "Expected an identifier, not '{}'!", lexer.get_current_lexeme()))
      return ErrorExpr::CreateExpr(ctx);
    if (check_and_consume(TKN_LEFT_PAREN, &ASTMaker::panic_consume_fn_decl,
      "Expected a '('!"))
      return ErrorExpr::CreateExpr(ctx);

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

    if (check_and_consume(TKN_RIGHT_PAREN, &ASTMaker::panic_consume_fn_decl,
      "Expected a ')'!"))
      return ErrorExpr::CreateExpr(ctx);
    if (check_and_consume(TKN_MINUS_GREAT, &ASTMaker::panic_consume_fn_decl,
      "Expected a '->'!"))
      return ErrorExpr::CreateExpr(ctx);

    //The return type of the function
    PTR<const Type> return_t = parse_typename(&ASTMaker::panic_consume_fn_decl);
    if (is_a<ErrorType>(return_t))
      return ErrorExpr::CreateExpr(ctx);

    PTR<const Type> fn_ptr_t = FnType::CreateFn(return_t, std::move(args_type), is_vararg, ctx);
    PTR<FnDeclExpr> declaration = as<PTR<FnDeclExpr>>(FnDeclExpr::CreateExpr(fn_ptr_t, fn_name, std::move(args_name), is_extern, line_state.to_src_info(), ctx));

    //Set the current function being parsed
    current_function = declaration;
    //And reset it on scope exit
    ON_EXIT{ current_function = nullptr; };

    //If 'main' function, check declaration
    if (declaration->is_main()
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
      if (!current_function->get_return_type()->is_void() && !declaration->is_main())
        validate_all_path_return(body);
      //If a return is not present at the end of the void function,
      //add one. As parse_scope can return ErrorExpr or ScopeExpr,
      //do necessary check
      else if (is_a<ScopeExpr>(body) && !isTerminatedExpr(body))
      {
        //If main has no return, add 'return 0'
        //If function is not main, (and returns void) add 'return void'
        as<PTR<ScopeExpr>>(body)->push_back(FnReturnExpr::CreateExpr(
          !declaration->is_main() ? nullptr :
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
      return parse_var_decl(false);
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

    PTR<Expr> if_cond = parse_bin_cond(); //if condition    
    PTR<Expr> if_body = parse_scope(); //if body

    if (current_tkn == TKN_KEYWORD_ELIF)
    {
      //Little trick: an elif token is parsed as being
      //an else if.
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
    //Propagate error
    if (is_a<ErrorExpr>(if_cond))
      return if_cond;

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

    PTR<Expr> condition = parse_bin_cond();
    PTR<Expr> body = parse_scope();
    if (isTerminatedExpr(body))
    {
      generate_any<report_as::WARNING>(body->get_src_code(), nullptr,
        "Loop body is terminated!");
    }

    //Restore loop state
    is_parsing_loop = old_is_loop;

    //Propagate error
    if (is_a<ErrorExpr>(condition))
      return condition;

    return WhileLoopExpr::CreateExpr(condition, body,
      line_state.to_src_info(), ctx);
  }

  PTR<Expr> ASTMaker::parse_var_decl(bool is_global) noexcept
  {
    SavedExprInfo line_state = { *this };

    if (check_and_consume(TKN_KEYWORD_VAR, &ASTMaker::panic_consume_var_decl,
      "Expected a variable declaration!"))
      return ErrorExpr::CreateExpr(ctx);
    bool is_var_const = true;
    if (current_tkn == TKN_KEYWORD_MUT)
    {
      is_var_const = false;
      consume_current_tkn();
    }
    if (check_and_consume(TKN_IDENTIFIER, &ASTMaker::panic_consume_var_decl,
      "Expected an identifier!"))
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
      if (check_and_consume(TKN_EQUAL, &ASTMaker::panic_consume_var_decl, "Expected a '='!"))
        return save_var_decl(is_global, ErrorType::CreateType(ctx), var_name,
          var_init, line_state.to_src_info());
      var_init = parse_binary();
    }
    else if (var_type == nullptr)
    {
      generate_any<report_as::ERROR>(line_state.to_src_info(), &ASTMaker::panic_consume_var_decl,
        "An uninitialized variable should specify its type!");
      return save_var_decl(is_global, ErrorType::CreateType(ctx), var_name,
        var_init, line_state.to_src_info());
    }
    else //uninitialized variable with explicit type
    {
      consume_current_tkn();
      goto SAVE; //skip type conversions
    }

    //If the type is not explicit, deduce it from left hand side
    //else convert left hand side to explicit type
    if (var_type == nullptr)
      var_type = var_init->get_type();
    
    if (is_var_const)
      var_type = var_init->get_type()->clone_as_const(ctx);
    else
      var_type = var_init->get_type()->clone_as_mut(ctx);

    var_init = as_convert_to(var_init, var_type);

    if (check_and_consume(TKN_SEMICOLON, &ASTMaker::panic_consume_var_decl, "Expected a ';'!"))
      return save_var_decl(is_global, var_type, var_name,
        var_init, line_state.to_src_info());

  SAVE:
    return save_var_decl(is_global, var_type, var_name,
      var_init, line_state.to_src_info());
  }

  PTR<Expr> ASTMaker::parse_assignment(PTR<Expr> lhs, const SavedExprInfo& line_state) noexcept
  {
    Token assignment_tkn = current_tkn;
    consume_current_tkn(); //consume '='
    PTR<Expr> rhs = parse_binary();

    if (is_a<ErrorExpr>(lhs))
      return lhs;

    if (is_a<VarReadExpr>(lhs))
    {
      if (lhs->get_type()->is_const())
      {
        generate_any<report_as::ERROR>(lhs->get_src_code(), nullptr,
          "Cannot assign to a non-mutable variable!");
        return ErrorExpr::CreateExpr(ctx);
      }
    }
    else if (is_a<PtrLoadExpr>(lhs))
    {
      auto read = as<PTR<const PtrLoadExpr>>(lhs);
      //The type of the PtrLoadExpr is the type
      //pointed to. So we can directly check for const here.
      if (read->get_type()->is_const())
      {
        generate_any<report_as::ERROR>(lhs->get_src_code(), nullptr,
          "Cannot write through pointer ('{}') to non-mutable type!",
          read->get_ptr_type()->get_name());
        return ErrorExpr::CreateExpr(ctx);
      }
    }
    else
    {
      //No need to consume as the whole expression was already parsed.
      generate_any<report_as::ERROR>(lhs->get_src_code(), nullptr,
        "Left hand side of an assignment should be a variable!");
      return ErrorExpr::CreateExpr(ctx);
    }


    if (assignment_tkn == TKN_EQUAL)
    {
      if (!is_a<PtrLoadExpr>(lhs))
        return VarWriteExpr::CreateExpr(as<PTR<VarReadExpr>>(lhs), rhs,
          line_state.to_src_info(), ctx);
      return PtrStoreExpr::CreateExpr(as<PTR<PtrLoadExpr>>(lhs)->get_where(), rhs,
        line_state.to_src_info(), ctx);
    }
    
    //If not =, we transform the expression into an expanded version:
    //i += 1 -> i = i + 1
    auto write_val = create_binary(lhs->get_type(), lhs,
      DirectAssignToNonAssignToken(assignment_tkn),
      rhs, line_state.to_src_info());
    
    //Expand the direct assignment operator
    if (!is_a<PtrLoadExpr>(lhs))
      return VarWriteExpr::CreateExpr(as<PTR<VarReadExpr>>(lhs),
          write_val, line_state.to_src_info(), ctx);    
    return PtrStoreExpr::CreateExpr(as<PTR<PtrLoadExpr>>(lhs)->get_where(), write_val,
      line_state.to_src_info(), ctx);
  }

  PTR<Expr> ASTMaker::parse_conversion(PTR<Expr> lhs, const SavedExprInfo& line_state) noexcept
  {
    assert(current_tkn == TKN_KEYWORD_AS || current_tkn == TKN_KEYWORD_BIT_AS);

    auto cnv = current_tkn;

    consume_current_tkn();
    PTR<const Type> cnv_type = parse_typename();
    
    //Propagate error
    if (is_a<ErrorExpr>(lhs))
      return lhs;
    
    if (cnv == TKN_KEYWORD_BIT_AS)
      return ConvertExpr::CreateExpr(cnv_type, lhs, TKN_KEYWORD_BIT_AS,
        lhs->get_src_code(), ctx);

    return as_convert_to(lhs, cnv_type);
  }

  PTR<const Type> ASTMaker::parse_typename(panic_consume_t panic) noexcept
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
      if (!is_const && !is_parsing_ptr)
      {
        generate_any<report_as::ERROR>(line_state.to_src_info(), panic,
          "'void' typename cannot be marked as mutable!");
        return ErrorType::CreateType(ctx);
      }
      consume_current_tkn(); //void
      return VoidType::CreateType(ctx);
    }
    case TKN_KEYWORD_BOOL:
      consume_current_tkn();
      return BuiltInType::CreateBool(is_const, ctx);
    case TKN_KEYWORD_CHAR:
      consume_current_tkn();
      return BuiltInType::CreateChar(is_const, ctx);
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
    case TKN_KEYWORD_BYTE:
      consume_current_tkn();
      return BuiltInType::CreateBYTE(is_const, ctx);
    case TKN_KEYWORD_WORD:
      consume_current_tkn();
      return BuiltInType::CreateWORD(is_const, ctx);
    case TKN_KEYWORD_DWORD:
      consume_current_tkn();
      return BuiltInType::CreateDWORD(is_const, ctx);
    case TKN_KEYWORD_QWORD:
      consume_current_tkn();
      return BuiltInType::CreateQWORD(is_const, ctx);
    case TKN_KEYWORD_LSTRING:
    {
      if (!is_const)
      {
        generate_any<report_as::ERROR>(line_state.to_src_info(), panic,
          "'lstring' typename cannot be marked as mutable!");
        return ErrorType::CreateType(ctx);
      }
      consume_current_tkn();
      return BuiltInType::CreateLString(ctx);
    }
    case TKN_KEYWORD_PTR:
    {
      consume_current_tkn();
      if (!check_and_consume(TKN_LESS, panic, "Expected a '<'!"))
      {
        ScopedSave sv(is_parsing_ptr, true);
        
        PTR<const Type> ptr_to = parse_typename(panic);
        if (current_tkn == TKN_GREAT_GREAT) // '>>' is parsed as '>' '>'
          current_tkn = TKN_GREAT;
        if (!check_and_consume(TKN_GREAT, panic, "Expected a '>'!"))
          return PtrType::CreatePtr(is_const, ptr_to, ctx);
      }
    }
    break;
    case TKN_IDENTIFIER:
      //TODO: add
      colt_unreachable("not implemented");
    default:
      generate_any<report_as::ERROR>(line_state.to_src_info(), panic,
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
      return parse_fn_call(identifier, line_state);

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
      if (!is_a<VarDeclExpr>(gvar->second.get_front()))
      {
        generate_any<report_as::ERROR>(identifier_info, nullptr,
          "'{}' is not a variable!", identifier);
        return ErrorExpr::CreateExpr(ctx);
      }
      return VarReadExpr::CreateExpr(gvar->second.get_front()->get_type(), identifier,
        line_state.to_src_info(), ctx);
    }
    else
    {
      generate_any<report_as::ERROR>(identifier_info, nullptr,
        "Variable of name '{}' does not exist!", identifier);
      return ErrorExpr::CreateExpr(ctx);
    }
  }

  PTR<Expr> ASTMaker::parse_fn_call(StringView identifier, const SavedExprInfo& line_state) noexcept
  {
    assert(current_tkn == TKN_LEFT_PAREN);

    SourceCodeExprInfo identifier_location = line_state.to_src_info();

    Vector<PTR<Expr>> outer_scope = {};

    SmallVector<PTR<Expr>, 4> arguments;
    parse_parenthesis(&ASTMaker::parse_fn_call_args, arguments, outer_scope);

    auto call_expr = handle_function_call(identifier, std::move(arguments),
      identifier_location, line_state.to_src_info());
    if (is_a<ErrorExpr>(call_expr) || outer_scope.is_empty())
      return call_expr;
    outer_scope.push_back(call_expr);
    return ScopeExpr::CreateExpr(std::move(outer_scope),
      line_state.to_src_info(), ctx);
  }

  void ASTMaker::parse_fn_call_args(SmallVector<PTR<Expr>, 4>& arguments, Vector<PTR<Expr>>& scope) noexcept
  {
    if (current_tkn != TKN_RIGHT_PAREN)
    {
      auto expr = parse_binary();
      //Print(hello()) with Print(void) and hello()->void
      //becomes: hello(); Print();
      if (expr->get_type()->is_void())
        scope.push_back(expr);
      else
        arguments.push_back(expr);
    }
    while (current_tkn != TKN_RIGHT_PAREN)
    {
      if (check_and_consume(TKN_COMMA, "Expected a ')'!"))
        break;

      auto expr = parse_binary();
      if (expr->get_type()->is_void())
        scope.push_back(expr);
      else
        arguments.push_back(expr);
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
    PTR<Expr> ret_val = as_convert_to(parse_binary(),
      current_function->get_return_type());
    if (is_a<ErrorExpr>(ret_val))
    {
      generate_any<report_as::ERROR>(ret_val->get_src_code(), nullptr,
        "Type of return value does not match function return type!");
      return ret_val;
    }
    //Return the FnReturnExpr
    ret_val = FnReturnExpr::CreateExpr(ret_val, line_state.to_src_info(), ctx);
    check_and_consume(TKN_SEMICOLON, &ASTMaker::panic_consume_sttmnt,
      "Expected a ';'!");
    return ret_val;
  }

  bool ASTMaker::validate_fn_call(const SmallVector<PTR<Expr>, 4>& arguments, PTR<const FnDeclExpr> decl, StringView identifier, const SourceCodeExprInfo& info) noexcept
  {
    if (arguments.get_size() != decl->get_params_count())
    {
      if (decl->get_type()->is_varargs())
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
      if (is_a<ErrorExpr>(as_convert_to(arguments[i], decl->get_params_type()[i])))
      {
        generate_any<report_as::ERROR>(arguments[i]->get_src_code(), nullptr,
          "Type of argument ('{}') does not match that of declaration ('{}')!",
          arguments[i]->get_type()->get_name(), decl->get_params_type()[i]->get_name());
        ret = false;
      }
    }
    return ret;
  }

  PTR<Expr> ASTMaker::handle_function_call(StringView identifier, SmallVector<PTR<Expr>, 4>&& arguments, const SourceCodeExprInfo& identifier_loc, const SourceCodeExprInfo& fn_call) noexcept
  {
    auto ptr = global_map.find(identifier);
    if (ptr == nullptr)
    {
      generate_any<report_as::ERROR>(identifier_loc, nullptr,
        "Function of name '{}' does not exist!", identifier);
      return ErrorExpr::CreateExpr(ctx);
    }
    if (is_a<VarDeclExpr>(ptr->second.get_front()))
    {
      generate_any<report_as::ERROR>(identifier_loc, nullptr,
        "'{}' is a global variable, not a function!", identifier);
      return ErrorExpr::CreateExpr(ctx);
    }

    if (ptr->second.get_size() == 1)
    {
      if (auto decl = as<PTR<FnDefExpr>>(ptr->second.get_front())->get_fn_decl();
        validate_fn_call(arguments, decl, identifier, fn_call))
        return FnCallExpr::CreateExpr(decl, std::move(arguments), fn_call, ctx);
      return ErrorExpr::CreateExpr(ctx);
    }

    SmallVector<PTR<const FnDefExpr>> overload_set;
    for (auto i : ptr->second)
    {
      assert_true(is_a<FnDefExpr>(i), "Invalid global table entry!");
      auto fn = as<PTR<const FnDefExpr>>(i);
      if (fn->get_params_count() == arguments.get_size())
        overload_set.push_back(fn);
    }
    PTR<const FnDefExpr> best = nullptr;
    for (auto fn : overload_set)
    {
      for (size_t i = 0; i < fn->get_params_count(); i++)
      {
        if (!arguments[i]->get_type()->is_equal(fn->get_params_type()[i]))
          goto REPEAT;
      }
      best = fn;

    REPEAT:
      continue;
    }
    if (best == nullptr)
    {
      generate_any<report_as::ERROR>(identifier_loc, nullptr,
        "None of the overloads of function '{}' matches these arguments!", identifier);
      return ErrorExpr::CreateExpr(ctx);
    }
    return FnCallExpr::CreateExpr(best->get_fn_decl(),
      std::move(arguments), fn_call, ctx);
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

  PTR<Expr> ASTMaker::save_var_decl(bool is_global, PTR<const Type> var_type, StringView var_name, PTR<Expr> var_init, const SourceCodeExprInfo& src_info) noexcept
  {
    if (is_global)
    {
      auto var_expr = VarDeclExpr::CreateExpr(var_type, var_name, var_init, true,
        src_info, ctx);
      if (auto gptr = global_map.find(var_name); gptr == nullptr)
      {
        SmallVector<PTR<Expr>> to_push;
        to_push.push_back(var_expr);
        //TODO: move
        global_map.insert(var_name, to_push);
        return var_expr;
      }
      else
      {
        if (is_a<FnDefExpr>(gptr->second.get_front()))
          generate_any<report_as::ERROR>(src_info, nullptr,
            "Function of name '{}' already exist!", var_name);
        else
          generate_any<report_as::ERROR>(src_info, nullptr,
            "Global variable of name '{}' already exist!", var_name);
        return ErrorExpr::CreateExpr(ctx);
      }
    }

    local_var_table.push_back({ var_name, var_type });
    return VarDeclExpr::CreateExpr(var_type, var_name, var_init, false,
      src_info, ctx);
  }

  void ASTMaker::add_fn_to_global_table(PTR<FnDefExpr> expr) noexcept
  {
    auto ptr = global_map.find(expr->get_name());
    if (ptr == nullptr)
    {
      SmallVector<PTR<Expr>> exprs;
      exprs.push_back(expr);
      //TODO: move
      global_map.insert(expr->get_name(), exprs);
      return;
    }
    if (is_a<VarDeclExpr>(ptr->second.get_front()))
    {
      generate_any<report_as::ERROR>(expr->get_src_code(), nullptr,
        "Global variable of name '{}' already exist!", expr->get_name());
      return;
    }
    if (as<PTR<FnDefExpr>>(ptr->second.get_front())->is_extern())
    {
      if (expr->is_extern())
      {
        if (!expr->get_type()->is_equal(ptr->second.get_front()->get_type()))
        {
          generate_any<report_as::ERROR>(expr->get_src_code(), nullptr,
            "Cannot overload 'extern' functions!", expr->get_name());
        }
        return;
      }
      generate_any<report_as::ERROR>(expr->get_src_code(), nullptr,
        "Cannot overload non-'extern' with 'extern' functions!", expr->get_name());
      return;
    }

    for (auto i : ptr->second)
    {
      assert_true(is_a<FnDefExpr>(i), "Invalid global table entry!");
      auto fn = as<PTR<const FnDefExpr>>(i);
      //If the functions do not have the same number of arguments
      //then we can safely overload
      if (fn->get_params_count() != expr->get_params_count())
        continue;
      for (size_t j = 0; j < fn->get_params_count(); j++)
      {
        //If any of the arguments differ, we can safely overload
        if (!fn->get_params_type()[j]->is_equal(expr->get_params_type()[j]))
          goto REPEAT;
      }
      //Same function: if both have body, then error
      if (fn->get_return_type()->is_equal(expr->get_return_type()))
      {
        if (fn->has_body() && expr->has_body())
        {
          generate_any<report_as::ERROR>(expr->get_src_code(), nullptr,
            "Function of name '{}' already has a body!", expr->get_name());
          return;
        }
      }
      else //same arguments but different return types
      {
        generate_any<report_as::ERROR>(expr->get_src_code(), nullptr,
          "Cannot overload functions solely on return type!", expr->get_name());
        return;
      }

    REPEAT:
      continue;
    }
    //push new overload
    ptr->second.push_back(expr);
  }

  PTR<Expr> ASTMaker::create_binary(PTR<const Type> expr_type, PTR<Expr> lhs, Token op, PTR<Expr> rhs, const SourceCodeExprInfo& src_info) noexcept
  {
    BinaryOperator bin_op = TokenToBinaryOperator(op);
    //Type checks, and supported operators check
    if (!rhs->get_type()->is_equal(lhs->get_type()))
    {
      generate_any<report_as::ERROR>(src_info, &ASTMaker::panic_consume_semicolon,
        "Operands should be of same type!");
      return ErrorExpr::CreateExpr(ctx);
    }
    else if (bin_op != BinaryOperator::OP_ASSIGN && is_a<BuiltInType>(rhs->get_type())
      && !as<PTR<const BuiltInType>>(rhs->get_type())->supports(bin_op))
    {
      generate_any<report_as::ERROR>(src_info, &ASTMaker::panic_consume_semicolon,
        "Type '{}' does not support operator '{}'!", rhs->get_type()->get_name(), BinaryOperatorToString(bin_op));
      return ErrorExpr::CreateExpr(ctx);
    }

    //Check for division by zero and constant fold expression
    //if possible.
    if (is_a<LiteralExpr>(rhs))
    {
      auto rhs_l = as<PTR<const LiteralExpr>>(rhs);
      if (is_a<LiteralExpr>(lhs))
      {
        //Constant fold as both expression are known at compile-time
        //'constant_fold' also emits errors and warnings.
        return constant_fold(as<PTR<LiteralExpr>>(lhs), bin_op, rhs_l,
          as<PTR<const BuiltInType>>(expr_type), src_info);
      }
      //Detect division by zero
      else if ((bin_op == BinaryOperator::OP_DIV || bin_op == BinaryOperator::OP_MOD)
        && rhs_l->get_value().as<u64>() == 0
        && rhs_l->get_type()->is_integral())
      {
        generate_any<report_as::ERROR>(src_info, nullptr,
          "Integral division by zero is not allowed!");
        return ErrorExpr::CreateExpr(ctx);
      }
    }
    return BinaryExpr::CreateExpr(expr_type, lhs, op, rhs, src_info, ctx);
  }

  PTR<Expr> ASTMaker::create_binary(PTR<Expr> lhs, Token op, PTR<Expr> rhs, const SourceCodeExprInfo& src_info) noexcept
  {
    return create_binary(lhs->get_type(), lhs, op, rhs, src_info);
  }
  
  PTR<Expr> ASTMaker::constant_fold(PTR<const LiteralExpr> a, BinaryOperator op, PTR<const LiteralExpr> b, PTR<const BuiltInType> ret, const SourceCodeExprInfo& src_info) noexcept
  {
    //If the expression is 2 lstring to add, create lstring
    //that represents the concatenation of both arguments
    if (ret->is_lstring() && op == BinaryOperator::OP_SUM)
    {
      String concat = { *a->get_value().as<PTR<String>>() };
      concat += *b->get_value().as<PTR<String>>();
      QWORD res = str_table.insert(std::move(concat)).first;
      return LiteralExpr::CreateExpr(res, ret, src_info, ctx);
    }

    //We take advantage of the interpreter's instructions.
    //See "interpreter/qword_op.h"
    auto fn = op::getInstFromBinaryOperator(op);
    auto [res, err] = fn(a->get_value(), b->get_value(), a->get_type()->get_builtin_id());    
    
    if (err == op::DIV_BY_ZERO)
    {
      generate_any<report_as::ERROR>(src_info, nullptr,
        "Integral division by zero is not allowed!");
      return ErrorExpr::CreateExpr(ctx);
    }
    else if (err != op::NO_ERROR)
    {
      generate_any<report_as::WARNING>(src_info, nullptr,
        "{}", op::OpErrorToStrExplain(err));
    }
    return LiteralExpr::CreateExpr(res, ret, src_info, ctx);
  }

  void ASTMaker::panic_consume_semicolon() noexcept
  {
    while (current_tkn != TKN_SEMICOLON && current_tkn != TKN_RIGHT_CURLY
      && current_tkn != TKN_RIGHT_PAREN && current_tkn != TKN_EOF)
      consume_current_tkn();
  }

  void ASTMaker::panic_consume_decl() noexcept
  {
    while (current_tkn != TKN_KEYWORD_VAR && current_tkn != TKN_KEYWORD_FN && current_tkn != TKN_EOF)
      consume_current_tkn();
  }

  PTR<Expr> ASTMaker::parse_bin_cond() noexcept
  {
    PTR<Expr> condition = parse_binary();
    if (!condition->get_type()->is_equal(BuiltInType::CreateBool(false, ctx)))
    {
      generate_any<report_as::ERROR>(condition->get_src_code(), nullptr,
        "Expression should be of type 'bool'!");
      return ErrorExpr::CreateExpr(ctx);
    }
    //If the expression is not a comparison, but is of type bool
    //(read from boolean variable, ...), transform it
    //into a comparison with 'true'
    else if (!is_a<BinaryExpr>(condition))
    {
      condition = create_binary(condition->get_type(), condition, TKN_EQUAL_EQUAL,
        LiteralExpr::CreateValue(true, ctx), condition->get_src_code());
    }
    return condition;
  }

  void ASTMaker::panic_consume_return() noexcept
  {
    while (current_tkn != TKN_SEMICOLON && current_tkn != TKN_RIGHT_CURLY && current_tkn != TKN_EOF
      && current_tkn != TKN_KEYWORD_IF && current_tkn != TKN_KEYWORD_WHILE && current_tkn != TKN_KEYWORD_VAR)
      consume_current_tkn();
  }

  PTR<Expr> ASTMaker::as_convert_to(PTR<Expr> what, PTR<const Type> to) noexcept
  {
    PTR<const Type> from = what->get_type();
    if (from->is_lstring() && !to->is_builtin())
    {
      if (!as<PTR<const PtrType>>(to)->get_type_to()->is_char())
      {
        generate_any<report_as::ERROR>(what->get_src_code(), nullptr,
          "'lstring' can only be converted to a 'PTR<char>', not '{}'!", to->get_name());
        return ErrorExpr::CreateExpr(ctx);
      }
      return what;
    }
    if (from->is_builtin() && to->is_builtin())
    {
      if (from->is_equal(to))
        return what;
      if (from->is_lstring() || to->is_lstring())
      {
        generate_any<report_as::ERROR>(what->get_src_code(), nullptr,
          "Cannot convert '{}' to 'lstring'!", from->get_name());
        return ErrorExpr::CreateExpr(ctx);
      }
      //Create conversion.
      return ConvertExpr::CreateExpr(to, what, TKN_KEYWORD_AS,
        what->get_src_code(), ctx);
    }
    if (from->is_ptr() && to->is_ptr()) //both are pointers
    {
      auto to_p = as<PTR<const PtrType>>(to);
      auto from_p = as<PTR<const PtrType>>(from);
      if (!from_p->get_type_to()->is_equal(to_p->get_type_to()))
      {
        generate_any<report_as::ERROR>(what->get_src_code(), nullptr,
          "Cannot convert from '{}' to '{}'!",
          from->get_name(), to->get_name());
        return ErrorExpr::CreateExpr(ctx);
      }
      if (!to_p->get_type_to()->is_const()
        && from_p->get_type_to()->is_const())
      {
        generate_any<report_as::ERROR>(what->get_src_code(), nullptr,
          "Cannot convert from non-mutable '{}' to mutable pointer '{}'!",
          from->get_name(), to->get_name());
        return ErrorExpr::CreateExpr(ctx);
      }
    }
    return what;
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