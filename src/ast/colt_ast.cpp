/** @file colt_ast.cpp
* Contains definition of functions declared in 'colt_ast.h'.
*/

#include "colt_ast.h"

/// @brief If 'boolean' evaluates to true, goto label
#define IF_TRUE_GOTO(boolean, label) if ((boolean)) goto label
/// @brief If 'boolean' evaluates to false, goto label
#define IF_FALSE_GOTO(boolean, label) if (!(boolean)) goto label
/// @brief If 'boolean' evaluates to true, returns ErrorExpr::CreateExpr(ctx)
#define IF_TRUE_RET_ERR(boolean) if ((boolean)) return ErrorExpr::CreateExpr(ctx)

namespace colt::lang
{
  Expected<AST, u32> CreateAST(StringView from, COLTContext& ctx) noexcept
  {
    ASTMaker ast = { from, ctx };
    if (ast.is_empty() || ast.get_error_count() != 0)
      return { Error, ast.get_error_count() };
    else
      return { InPlace, ast.steal_result(), ctx };
  }

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

  bool isComparisonToken(Token tkn) noexcept
  {
    return TKN_GREAT_GREAT < tkn && tkn < TKN_EQUAL;
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

    //Post-unary operators
    if (is_valid_post_unary())
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
      generate_any_current<report_as::ERROR>(&ASTMaker::panic_consume_semicolon, "Expected a binary operator!");
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
        generate_any<report_as::ERROR>(line_state.to_src_info(), nullptr,
          "Operands should be of same type!");

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
      return parse_primary();
    
    //Dereference operator
    if (op == TKN_STAR)
    {
      auto expr = parse_primary();
      if (!is_a<VarReadExpr>(expr))
      {
        generate_any<report_as::ERROR>(expr->get_src_code(), nullptr,
          "Dereference operator '*' can only be applied on variables!");
        return ErrorExpr::CreateExpr(ctx);
      }
      if (!expr->get_type()->is_ptr())
      {
        generate_any<report_as::ERROR>(expr->get_src_code(), nullptr,
          "Dereference operator '*' can only be applied on pointer types!");
        return ErrorExpr::CreateExpr(ctx);
      }
      else
      {
        return UnaryExpr::CreateExpr(as<PTR<const PtrType>>(expr->get_type())->get_type_to(),
          current_tkn, false, expr, line_state.to_src_info(), ctx);
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
    
    IF_TRUE_RET_ERR(
      check_and_consume(TKN_IDENTIFIER, &ASTMaker::panic_consume_fn_decl, 
        "Expected an identifier, not '{}'!", fn_name)
    );
    IF_TRUE_RET_ERR(
      check_and_consume(TKN_LEFT_PAREN, &ASTMaker::panic_consume_fn_decl,
        "Expected a '('!")
    );
    
    SmallVector<PTR<const Type>, 4> args_type;
    SmallVector<StringView, 4> args_name;
    while (current_tkn != TKN_EOF && current_tkn != TKN_RIGHT_PAREN)
    {
      SavedExprInfo line_state_arg = { *this };

      args_type.push_back(parse_typename());
      auto arg_name = lexer.get_parsed_identifier();
      if (check_and_consume(TKN_IDENTIFIER, "Expected an identifier!"))
      {
        panic_consume_rparen();
        break;
      }
      
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
    SavedExprInfo line_state = { *this };
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
        generate_any<report_as::ERROR>(lexeme_info.to_src_info(), nullptr,
          "Unclosed curly bracket delimiter!");
      else //consume '}'
        consume_current_tkn();
    }
    else
      generate_any_current<report_as::ERROR>(nullptr, "Expected the beginning of a scope ('{{'{}", one_expr ? "or ':')!" : ")!");
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
      generate_any_current<report_as::ERROR>(nullptr, "Expected a statement!");
      return ErrorExpr::CreateExpr(ctx);
    default: break;
    }
    auto to_ret = parse_binary();
    check_and_consume(TKN_SEMICOLON, "Expected a ';'!");
    return to_ret;
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
    else
    {
      consume_current_tkn();
      goto GEN;
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
    PTR<Expr> rhs = parse_binary();

    if (!is_a<VarReadExpr>(lhs))
    {
      generate_any<report_as::ERROR>(lhs->get_src_code(), nullptr,
        "Left hand side of an assignment should be a variable!");
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
    
    bool is_mut = false;
    if (current_tkn == TKN_KEYWORD_MUT) // mut TYPE
    {
      is_mut = true;
      consume_current_tkn();
    }

    switch (current_tkn)
    {
    case TKN_KEYWORD_VOID:
    {
      if (is_mut)
        generate_any<report_as::ERROR>(line_state.to_src_info(), nullptr,
          "'void' typename cannot be marked as mutable!");
      return VoidType::CreateType(ctx);
    }
    case TKN_KEYWORD_BOOL:
      consume_current_tkn();
      return BuiltInType::CreateBool(is_mut, ctx);
    case TKN_KEYWORD_CHAR:
      //TODO: add
      colt_unreachable("not implemented");
      //return BuiltInType::CreateChar(is_mut, ctx);
    case TKN_KEYWORD_I8:
      consume_current_tkn();
      return BuiltInType::CreateI8(is_mut, ctx);
    case TKN_KEYWORD_U8:
      consume_current_tkn();
      return BuiltInType::CreateU8(is_mut, ctx);
    case TKN_KEYWORD_I16:
      consume_current_tkn();
      return BuiltInType::CreateI16(is_mut, ctx);
    case TKN_KEYWORD_U16:
      consume_current_tkn();
      return BuiltInType::CreateU16(is_mut, ctx);
    case TKN_KEYWORD_I32:
      consume_current_tkn();
      return BuiltInType::CreateI32(is_mut, ctx);
    case TKN_KEYWORD_U32:
      consume_current_tkn();
      return BuiltInType::CreateU32(is_mut, ctx);
    case TKN_KEYWORD_I64:
      consume_current_tkn();
      return BuiltInType::CreateI64(is_mut, ctx);
    case TKN_KEYWORD_U64:
      consume_current_tkn();
      return BuiltInType::CreateU64(is_mut, ctx);
    case TKN_KEYWORD_FLOAT:
      consume_current_tkn();
      return BuiltInType::CreateF32(is_mut, ctx);
    case TKN_KEYWORD_DOUBLE:
      consume_current_tkn();
      return BuiltInType::CreateF64(is_mut, ctx);
    case TKN_KEYWORD_LSTRING:
      //TODO: add
      colt_unreachable("not implemented");
    case TKN_KEYWORD_PTR:
    {
      consume_current_tkn();
      if (!check_and_consume(TKN_LESS, "Expected a '<'!"))
      {
        PTR<const Type> ptr_to = parse_typename();
        if (current_tkn == TKN_GREAT_GREAT) // '>>' is parsed as '>' '>'
          current_tkn = TKN_GREAT;
        if (!check_and_consume(TKN_GREAT, "Expected a '>'!"))
          return PtrType::CreatePtr(is_mut, ptr_to, ctx);
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
      auto fn_param = current_function->get_params_name();
      //Search in arguments of function
      for (size_t i = 0; i < fn_param.get_size(); i++)
      {
        if (fn_param[i] == identifier)
          return VarReadExpr::CreateExpr(current_function->get_params_type()[i], identifier,
            line_state.to_src_info(), ctx);
      }
    }
    generate_any<report_as::ERROR>(identifier_info, nullptr,
      "Variable of name '{}' does not exist!", identifier);
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

  bool ASTMaker::validate_fn_call(const SmallVector<PTR<Expr>, 4>& arguments, PTR<const FnDeclExpr> decl, StringView identifier, const SourceCodeExprInfo& info) noexcept
  {
    if (arguments.get_size() != decl->get_params_name().get_size())
    {
      generate_any<report_as::ERROR>(info, nullptr, "Function '{}' expects {} argument{} not {}!", identifier,
        decl->get_params_name().get_size(), decl->get_params_name().get_size() == 1 ? "," : "s,", arguments.get_size());
      return false;
    }
    bool ret = true;
    for (size_t i = 0; i < arguments.get_size(); i++)
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
  
  void ASTMaker::panic_consume_semicolon() noexcept
  {
    while (current_tkn != TKN_SEMICOLON && current_tkn != TKN_EOF)
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
    while (current_tkn != TKN_SEMICOLON && is_valid_scope_begin() && current_tkn != TKN_EOF)
      consume_current_tkn();
  }
  
  void ASTMaker::panic_consume_rparen() noexcept
  {
    while (current_tkn != TKN_SEMICOLON && is_valid_scope_begin() && current_tkn != TKN_EOF)
      consume_current_tkn();
  }
}

#undef IF_TRUE_GOTO