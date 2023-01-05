/** @file colt_ast.h
* Contains the abstract syntax tree helpers.
* Use CreateAST to create an abstract syntax tree of a program.
*/

#ifndef HG_COLT_AST
#define HG_COLT_AST

#include "colt_expr.h"
#include "colt_context.h"
#include "parsing/colt_lexer.h"
#include "interpreter/qword_op.h"

namespace colt::lang
{
  /// @brief Returns the precedence of an operator or 255 if the token is not an operator.
  /// This function also returns a valid precedence for ')' or ',' or ';' or 'TKN_ERROR'.
  /// This simplifies Pratt's parsing in parse binary.
  /// @param tkn The Token whose precedence to check
  /// @return Precedence or 255 if not an operator
  u8 getOpPrecedence(Token tkn) noexcept;

  /// @brief Check if 'expr' is a terminating expression.
  /// A terminating expression is a 'return', a scope whose last
  /// expression is a 'return', or an if/else whose branches are terminated expressions.
  /// @param expr The expression to check for
  /// @return True if represents a return expression
  bool isTerminatedExpr(PTR<const Expr> expr) noexcept;

  /// @brief Concatenate two adjacent SourceCodeExprInfo
  /// @param lhs The left hand side
  /// @param rhs The right hand side
  /// @return Concatenated SourceCodeExprInfo
  SourceCodeExprInfo ConcatInfo(const SourceCodeExprInfo& lhs, const SourceCodeExprInfo& rhs) noexcept;
  
  //Forward declaration
  struct AST;

  /// @brief Class responsible of producing an AST
  class ASTMaker
  {
    /// @brief POD for line and expression informations of a single lexeme
    struct SourceCodeLexemeInfo
    {
      /// @brief The line number of the beginning of the current expression
      size_t line_nb;
      /// @brief StringView of the line containing the current expressions
      StringView line_strv;
      /// @brief StringView of the expression
      StringView expression;

      /// @brief Converts to a SourceCodeExprInfo
      /// @return The SourceCodeExprInfo
      SourceCodeExprInfo to_src_info() const noexcept;
    };

    /************* MEMBERS ************/

    /// @brief The array of expressions
    Vector<PTR<Expr>>& expressions;
    /// @brief The number of error that where generated
    u16 error_count = 0;
    /// @brief The number of warning that where generated
    u16 warn_count = 0;
    /// @brief The lexer responsible of breaking a StringView into tokens
    Lexer lexer;
    /// @brief The current token
    Token current_tkn;
    /// @brief True if parsing body of loop
    bool is_parsing_loop = false;
    /// @brief The table storing local variables informations
    Vector<std::pair<StringView, PTR<const Type>>> local_var_table = {};
    /// @brief The current expression informations
    SourceCodeLexemeInfo current_lexeme_info = {};
    /// @brief The last parsed lexeme informations
    SourceCodeLexemeInfo last_lexeme_info = {};
    /// @brief The current function being parsed
    PTR<const FnDeclExpr> current_function = nullptr;
    /// @brief Map responsible of storing global state (functions, global variables)
    Map<StringView, SmallVector<PTR<Expr>>>& global_map;
    /// @brief Table of String literals
    StableSet<String>& str_table;
    /// @brief The context storing types and expressions
    COLTContext& ctx;

    /************* STATE HANDLING HELPERS ************/

    /// @brief Helper for saving and restoring expressions informations.
    /// This class allows simplified generation of SourceCodeExprInfo for any expression.      
    struct SavedExprInfo
    {
      /// @brief The AST whose data to override and restore
      ASTMaker& ast;
      /// @brief The old AST's line informations
      SourceCodeLexemeInfo infos;

      //No copy constructor
      SavedExprInfo(const SavedExprInfo&) = delete;
      //No move constructor
      SavedExprInfo(SavedExprInfo&&) = delete;
      /// @brief Saves the SourceCodeLexemeInfo state of the ASTMaker
      /// @param ast The ASTMaker whose state to save
      SavedExprInfo(ASTMaker& ast) noexcept;
      /// @brief Restores the old ASTMaker's line informations
      ~SavedExprInfo() noexcept;

      /// @brief Transforms the current expression to a SourceCodeExprInfo.
      /// The reason to_src_info uses last_lexeme_info is because of 'current_tkn',
      /// which contains the NEXT token to consume, which means 'current_lexeme_info' contains
      /// information about the NEXT token which is not part of the current expression.
      /// @return Source code information about current expression
      SourceCodeExprInfo to_src_info() const noexcept;
    };

    /// @brief Helper for storing and restoring local variable table state
    struct SavedLocalState
    {
      /// @brief The ASTMaker whose local variable state to save
      ASTMaker& ast;
      /// @brief The old size of the local variable table
      size_t old_sz;

      //No copy constructor
      SavedLocalState(const SavedLocalState&) = delete;
      //No move constructor
      SavedLocalState(SavedLocalState&&) = delete;
      /// @brief Saves the local variable state of an ASTMaker
      /// @param ast The ASTMaker's state to save
      SavedLocalState(ASTMaker& ast) noexcept;
      /// @brief Restores the state of the ASTMaker
      ~SavedLocalState() noexcept;
    };

    /// @brief Get the current line informations
    /// @return The current line informations
    SourceCodeLexemeInfo get_expr_info() const noexcept;

    /// @brief Type of methods consuming tokens
    using panic_consume_t = void(ASTMaker::*)() noexcept;

  public:
    /// @brief Parses a StringView into an abstract syntax tree
    /// @param strv The StringView to parse (should be NUL-terminated)    
    /// @param ast The AST in which to store the result
    ASTMaker(StringView strv, AST& ast) noexcept;
    //No default move constructor
    ASTMaker(ASTMaker&&) = delete;
    //No default copy constructor
    ASTMaker(const ASTMaker&) = delete;

    /// @brief Returns the number of error generated
    /// @return The error count
    u16 get_error_count() const noexcept { return error_count; }
    /// @brief Returns the number of warning generated
    /// @return The warning count
    u16 get_warn_count() const noexcept { return warn_count; }

    /// @brief Check if the abstract syntax tree does not contain any expression
    /// @return True if the abstract syntax tree is empty
    bool is_empty() const noexcept { return expressions.get_size() == 0; }

  private:
    /// @brief Enum used to specify output type (for 'generate_any')
    enum class report_as
    {
      /// @brief Generates an error
      ERROR,
      /// @brief Generates a warning
      WARNING,
      /// @brief Generates a message
      MESSAGE
    };

    /// @brief Updates 'current_tkn' to the next token
    void consume_current_tkn() noexcept;

    /************* EXPRESSION PARSING ************/

    template<typename RetT, typename... Args> 
    RetT parse_parenthesis(RetT(ASTMaker::* method_ptr)(Args...), Args&&... args) noexcept;

    /// @brief Parses a LiteralExpr, VarReadExpr, FnCallExpr, or a UnaryExpr.
    /// @return Resulting expression or ErrorExpr on errors
    PTR<Expr> parse_primary() noexcept;

    /// @brief Parses a BinaryExpr, or a 'primary_expr'.
    /// @return Resulting expression or ErrorExpr on errors
    PTR<Expr> parse_binary(u8 precedence = 0) noexcept;

    /// @brief Parses a UnaryExpr.
    /// Precondition: current_tkn contains a UnaryOperator.
    /// @return Resulting expression or ErrorExpr on errors
    PTR<Expr> parse_unary() noexcept;

    /// @brief Parses a global declaration (FnDefExpr or VarDeclExpr).
    /// This function will return a FnDefExpr even if the parsed expression
    /// is a declaration without body.
    /// @return Resulting expression or ErrorExpr on errors
    PTR<Expr> parse_global_declaration() noexcept;

    /// @brief Parses a function declaration/definition (FnDefExpr).    
    /// Precondition: current_tkn == TKN_KEYWORD_FN
    /// This function will return a FnDefExpr even if the parsed expression
    /// is a declaration without body.
    /// @return Resulting expression or ErrorExpr on errors
    PTR<Expr> parse_fn_decl() noexcept;

    /// @brief Parses a scope.
    /// If 'one_expr' is true, accepts a single statement scope
    /// (starting with ':', not '{').
    /// Even only one statement is contained inside the scope, the returned
    /// expression will be a ScopeExpr.
    /// @return ScopeExpr or ErrorExpr
    PTR<Expr> parse_scope(bool one_expr = true) noexcept;

    /// @brief Parses a statement.
    /// @return Resulting expression or ErrorExpr
    PTR<Expr> parse_statement() noexcept;

    /// @brief Parses a conditional expression.
    /// Precondition: current_tkn == TKN_KEYWORD_IF
    /// @return ConditionExpr or ErrorExpr
    PTR<Expr> parse_condition() noexcept;

    /// @brief Parses a 'while' expression.
    /// Precondition: current_tkn == TKN_KEYWORD_WHILE
    /// @return WhileExpr or ErrorExpr
    PTR<Expr> parse_while() noexcept;

    /// @brief Parses a variable declaration (global or local)
    /// @param is_global True if the variable declaration should is global
    /// @return VarDeclExpr or ErrorExpr
    PTR<Expr> parse_variable_decl(bool is_global) noexcept;

    /// @brief Parses an assignment (=, or any self assignment)
    /// @param lhs The left hand side of the assignment
    /// @return VarWriteExpr or ErrorExpr
    PTR<Expr> parse_assignment(PTR<Expr> lhs, const SavedExprInfo& line_state) noexcept;

    /// @brief Parses a conversion (EXPR as TYPE)
    /// @param lhs The expression to convert
    /// @return ConvertExpr or ErrorExpr
    PTR<Expr> parse_conversion(PTR<Expr> lhs, const SavedExprInfo& line_state) noexcept;

    /// @brief Parses a typename (can be function pointer/pointer)
    /// @return Parsed typename or ErrorType
    PTR<const Type> parse_typename(panic_consume_t panic = nullptr) noexcept;

    /// @brief Handles an identifier in a primary expression.
    /// The identifier could represent a variable, or a function call
    /// @return VarReadExpr, FnCallExpr, or ErrorExpr
    PTR<Expr> parse_identifier(const SavedExprInfo& line_state) noexcept;

    /// @brief Handles a function call, with overload resolution
    /// @param identifier The function name
    /// @param line_state The line state from of the function calling this function
    /// @return FnCallExpr, or ErrorExpr
    PTR<Expr> parse_fn_call(StringView identifier, const SavedExprInfo& line_state) noexcept;

    /// @brief Parses a 'return' statement.
    /// Precondition: current_tkn == TKN_KEYWORD_RETURN.
    /// Does type checking with return type of current function.
    /// FnReturnExpr or ErrorExpr
    PTR<Expr> parse_return() noexcept;

    /// @brief Parses a binary expression of type bool
    /// @return BinaryExpr of type bool or ErrorExpr
    PTR<Expr> parse_bin_cond() noexcept;

    /// @brief Parses the function call's arguments
    /// @param arguments The argument vector in which to push the arguments
    void parse_fn_call_args(SmallVector<PTR<Expr>, 4>& arguments, Vector<PTR<Expr>>& scope) noexcept;

    /// @brief Validates a function call by doing type checking
    /// @param arguments The arguments passed to the function
    /// @param decl The declaration of the function being called
    /// @param identifier The identifier of the function
    /// @param info The function call source code information
    /// @return True if valid
    bool validate_fn_call(const SmallVector<PTR<Expr>, 4>& arguments, PTR<const FnDeclExpr> decl, StringView identifier, const SourceCodeExprInfo& info) noexcept;

    /// @brief Check recursively and prints errors if 'expr' does not end with a return
    void validate_all_path_return(PTR<const Expr> expr) noexcept;

    PTR<Expr> handle_function_call(StringView identifier, SmallVector<PTR<Expr>, 4>&& arguments, const SourceCodeExprInfo&  identifier_loc, const SourceCodeExprInfo& fn_call) noexcept;

    /// @brief Consumes statement till a RIGHT_CURLY_BRACKET is hit and generates a warning
    void handle_unreachable_code() noexcept;

    /// @brief Registers a function to the global table
    void add_fn_to_global_table(PTR<FnDefExpr> expr) noexcept;
    
    /************* FOLDING HELPERS ************/

    /// @brief Creates a binary expression, doing type checks and folding
    /// @param expr_type The resulting expression type
    /// @param lhs The left hand side of the expression
    /// @param op The binary operator
    /// @param rhs The right hand side of the expression
    /// @param src_info The source information of the whole expression
    /// @return BinaryExpr or ErrorExpr
    PTR<Expr> create_binary(PTR<const Type> expr_type, PTR<Expr> lhs, Token op, PTR<Expr> rhs, const SourceCodeExprInfo& src_info) noexcept;
    
    /// @brief Creates a binary expression, doing type checks and folding.
    /// This overload assume that the resulting expression will have the type
    /// of 'lhs' (and 'rhs' as type checks forbid both types be different).
    /// @param lhs The left hand side of the expression
    /// @param op The binary operator
    /// @param rhs The right hand side of the expression
    /// @param src_info The source information of the whole expression
    /// @return BinaryExpr or ErrorExpr
    PTR<Expr> create_binary(PTR<Expr> lhs, Token op, PTR<Expr> rhs, const SourceCodeExprInfo& src_info) noexcept;

    /// @brief Constant fold a binary expression.
    /// Constant folding is the process of moving computations with literals
    /// to the compilation. An expression like '10 + 5' is replaced by
    /// '15'. While this is a very simple optimization that is done directly
    /// by the backend, constant folding in the front-end allows better
    /// diagnostics (reporting division by zero as errors), and will be
    /// very useful for 'const' expressions.
    /// @param a The left hand side of the expression
    /// @param op The binary operator to fold
    /// @param b The right hand side of the expression
    /// @param ret The return type of the expression
    /// @param src_info The source informations of the whole expression
    /// @return LiteralExpr or ErrorExpr
    PTR<Expr> constant_fold(PTR<const LiteralExpr> a, BinaryOperator op, PTR<const LiteralExpr> b, PTR<const BuiltInType> ret, const SourceCodeExprInfo& src_info) noexcept;

    /// @brief Converts 'what' to type 'to', and prints error
    /// @param what The expression to convert
    /// @param to The type to convert to
    /// @return Converted expression or ErrorExpr on errors
    PTR<Expr> convert_to(PTR<Expr> what, PTR<const Type> to) noexcept;

    /************* PEEKING HELPERS ************/

    /// @brief Check if the current token is the beginning of a scope
    /// @return True if the current token is ':' or '{'
    bool is_valid_scope_begin() const noexcept { return current_tkn == TKN_COLON || current_tkn == TKN_LEFT_CURLY; }
    /// @brief Check if the current token is a post-unary operator
    /// @return True if the current token is '++' or '--'
    bool is_valid_post_unary() const noexcept { return current_tkn == TKN_PLUS_PLUS || current_tkn == TKN_MINUS_MINUS; }    

    /************* ERROR HANDLING HELPERS ************/

    /// @brief Consumes all tokens till a TKN_SEMICOLON or TKN_EOF is hit
    void panic_consume_semicolon() noexcept;
    /// @brief Consumes all tokens till a TKN_KEYWORD_VAR, TKN_KEYWORD_FN or TKN_EOF is hit
    void panic_consume_decl() noexcept;
    /// @brief Consumes all tokens till a TKN_KEYWORD_VAR/IF/WHILE/SEMICOLON/EOF is hit, consuming the SEMICOLON
    void panic_consume_sttmnt() noexcept;
    /// @brief Consumes all tokens till a TKN_SEMICOLON or TKN_EOF is hit and consumes the TKN_SEMICOLON
    void panic_consume_var_decl() noexcept;
    /// @brief Consumes all tokens till a TKN_SEMICOLON, TKN_EOF or a valid scope beginning is hit
    void panic_consume_fn_decl() noexcept;
    /// @brief Consumes all tokens till a TKN_RIGHT_PAREN or TKN_EOF is hit
    void panic_consume_rparen() noexcept;
    /// @brief Consumes the same tokens as panic_consume_sttmnt(), without consuming the SEMICOLON
    void panic_consume_return() noexcept;

    template<typename... Args>
    /// @brief Validates that the current token is 'expected' and consumes it, else generates 'error'
    /// @tparam ...Args The parameter pack to format
    /// @param expected The expected token
    /// @param fmt The error format to print if the current token does not match 'expected'
    /// @param ...args The argument pack to format
    /// @return True if the token was invalid
    bool check_and_consume(Token expected, fmt::format_string<Args...> fmt, Args&&... args) noexcept;

    template<typename... Args>
    /// @brief Validates that the current token is 'expected' and consumes it, else generates 'error'
    /// @tparam ...Args The parameter pack to format
    /// @param expected The expected token
    /// @param panic The method to consume tokens in case of errors
    /// @param fmt The error format to print if the current token does not match 'expected'
    /// @param ...args The argument pack to format
    /// @return True if the token was invalid
    bool check_and_consume(Token expected, panic_consume_t panic, fmt::format_string<Args...> fmt, Args&&... args) noexcept;

    template<report_as as, typename... Args>
    /// @brief Generates a message/warning/error using the 'src_info' and consumes tokens if required
    /// @tparam ...Args The arguments type to format
    /// @param src_info The source code information to pass to 'report_fn'
    /// @param panic_c The method to call after reporting (can be nullptr)
    /// @param report_fn The function to call (GenerateError, GenerateWarning...)
    /// @param fmt The format of the arguments
    /// @param ...args The arguments to format
    void generate_any(const SourceCodeExprInfo& src_info, panic_consume_t panic_c,
      fmt::format_string<Args...> fmt, Args&& ...args) noexcept;

    template<report_as as, typename... Args>
    /// @brief Generates a message/warning/error over the current lexeme and consumes tokens if required
    /// @tparam ...Args The arguments type to format
    /// @param panic_c The method to call after reporting (can be nullptr)
    /// @param report_fn The function to call (GenerateError, GenerateWarning...)
    /// @param fmt The format of the arguments
    /// @param ...args The arguments to format
    void generate_any_current(panic_consume_t panic_c, fmt::format_string<Args...> fmt,
      Args&& ...args) noexcept;
  };  
  
  /// @brief An abstract tree of a COLT program
  struct AST
  {
    /// @brief The array of expressions
    Vector<PTR<Expr>> expressions = {};
    /// @brief The global function/variable table
    Map<StringView, SmallVector<PTR<Expr>>> global_map = {};
    /// @brief The table of String literals
    StableSet<String> str_table = {};
    /// @brief The context storing type and expression informations
    COLTContext& ctx;

    /// @brief Creates an AST
    /// @param ctx The context storing the expressions
    AST(COLTContext& ctx) noexcept
      : ctx(ctx) {}
   };

  /// @brief Creates an Abstract Syntax Tree by parsing a StringView
  /// @param from The StringView to parse
  /// @param ctx The COLTContext where to store the expressions
  /// @return Error count if errors where detected else the AST
  Expected<AST, u32> CreateAST(StringView from, COLTContext& ctx) noexcept;
  /// @brief Compiles an expression and adds it to an already existing AST
  /// @param str The StringView to parse
  /// @param ast The AST in which to add the result
  /// @return True if compilation was successful
  bool CompileAndAdd(StringView str, AST& ast) noexcept;

  /************************************
  * IMPLEMENTATIONS
  ************************************/

  template<typename RetT, typename ...Args>
  RetT ASTMaker::parse_parenthesis(RetT(ASTMaker::* method_ptr)(Args...), Args&&... args) noexcept
  {
    auto info = lexer.get_line_info();
    //Construct source information from lexeme information
    SourceCodeExprInfo lexeme_info = { info.line_nb, info.line_nb, info.line_strv, lexer.get_current_lexeme() };

    check_and_consume(TKN_LEFT_PAREN, "Expected a '('!");
    if constexpr (std::is_same_v<RetT, void>)
    {
      (*this.*method_ptr)(std::forward<Args>(args)...);
      if (current_tkn != TKN_RIGHT_PAREN)
        generate_any<report_as::ERROR>(lexeme_info, &ASTMaker::panic_consume_semicolon, "Unclosed parenthesis delimiter!");
      else
        consume_current_tkn();
    }
    else
    {
      auto to_ret = (*this.*method_ptr)(std::forward<Args>(args)...); //Call method
      if (current_tkn != TKN_RIGHT_PAREN)
        generate_any<report_as::ERROR>(lexeme_info, &ASTMaker::panic_consume_semicolon, "Unclosed parenthesis delimiter!");
      else
        consume_current_tkn();
      return to_ret;
    }
  }

  template<typename ...Args>
  bool ASTMaker::check_and_consume(Token expected, fmt::format_string<Args...> fmt, Args && ...args) noexcept
  {
    if (current_tkn == expected)
    {
      consume_current_tkn();
      return false;
    }
    else
    {
      generate_any_current<report_as::ERROR>(&ASTMaker::panic_consume_semicolon, fmt, std::forward<Args>(args)...);
      return true;
    }
  }

  template<typename ...Args>
  bool ASTMaker::check_and_consume(Token expected, panic_consume_t panic, fmt::format_string<Args...> fmt, Args && ...args) noexcept
  {
    if (current_tkn == expected)
    {
      consume_current_tkn();
      return false;
    }
    else
    {
      generate_any_current<report_as::ERROR>(panic, fmt, std::forward<Args>(args)...);
      return true;
    }
  }

  template<ASTMaker::report_as as, typename ...Args>
  void ASTMaker::generate_any(const SourceCodeExprInfo& src_info, panic_consume_t panic_c,
    fmt::format_string<Args...> fmt, Args&&... args) noexcept
  {
    //Print using the right function
    if constexpr (as == report_as::ERROR)
    {
      GenerateError(src_info, fmt, std::forward<Args>(args)...);
      ++error_count;
    }
    else if constexpr (as == report_as::WARNING)
    {
      GenerateWarning(src_info, fmt, std::forward<Args>(args)...);
      ++warn_count;
    }
    else if constexpr (as == report_as::MESSAGE)
      GenerateMessage(src_info, fmt, std::forward<Args>(args)...);

    if (panic_c != nullptr)
      (*this.*panic_c)(); //call the panic function
  }

  template<ASTMaker::report_as as, typename ...Args>
  void ASTMaker::generate_any_current(panic_consume_t panic_c, fmt::format_string<Args...> fmt, Args&&... args) noexcept
  {
    auto info = lexer.get_line_info();
    //Construct source information from lexeme information
    SourceCodeExprInfo src_info = { info.line_nb, info.line_nb, info.line_strv, lexer.get_current_lexeme() };

    //Print using the right function
    if constexpr (as == report_as::ERROR)
    {
      GenerateError(src_info, fmt, std::forward<Args>(args)...);
      ++error_count;
    }
    else if constexpr (as == report_as::WARNING)
    {
      GenerateWarning(src_info, fmt, std::forward<Args>(args)...);
      ++warn_count;
    }
    else if constexpr (as == report_as::MESSAGE)
      GenerateMessage(src_info, fmt, std::forward<Args>(args)...);

    if (panic_c != nullptr)
      (*this.*panic_c)(); //call the panic function
  }
}

#endif //!HG_COLT_AST