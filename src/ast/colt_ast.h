/** @file colt_ast.h
* Contains the abstract syntax tree helpers.
* Use CreateAST to create an abstract syntax tree of a program.
*/

#ifndef HG_COLT_AST
#define HG_COLT_AST

#include "colt_expr.h"
#include "context/colt_context.h"
#include "parsing/colt_lexer.h"

namespace colt::lang
{
  namespace details
  {
    /// @brief Returns the precedence of an operator or 255 if the token is not an operator.
    /// This function also returns a valid precedence for ')' or ',' or ';' or 'TKN_ERROR'.
    /// This simplifies Pratt's parsing in parse binary.
    /// @param tkn The Token whose precedence to check
    /// @return Precedence or 255 if not an operator
    u8 GetOpPrecedence(Token tkn) noexcept;

    /// @brief Check if a Token represents any assignment Token (=, +=, ...)
    /// @param tkn The token to check for
    /// @return True if the Token is an assignment Token
    bool isAssignmentToken(Token tkn) noexcept;

    /// @brief Concatenate two adjacent SourceCodeExprInfo
    /// @param lhs The left hand side
    /// @param rhs The right hand side
    /// @return Concatenated SourceCodeExprInfo
    SourceCodeExprInfo ConcatInfo(const SourceCodeExprInfo& lhs, const SourceCodeExprInfo& rhs) noexcept;

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
      };

      /************* MEMBERS ************/

      /// @brief The array of expressions
      Vector<PTR<Expr>> expressions = {};
      /// @brief The number of error that where generated
      u16 error_count = 0;
      /// @brief The number of warning that where generated
      u16 warn_count = 0;
      /// @brief The lexer responsible of breaking a StringView into tokens
      Lexer lexer;
      /// @brief The current token
      Token current_tkn;
      /// @brief The table storing local variables informations
      Vector<std::pair<StringView, PTR<const Type>>> local_var_table = {};
      /// @brief The current expression informations
      SourceCodeLexemeInfo current_lexeme_info = {};
      /// @brief The last parsed lexeme informations
      SourceCodeLexemeInfo last_lexeme_info = {};
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

    public:
      /// @brief Parses a StringView into an abstract syntax tree
      /// @param strv The StringView to parse (should be NUL-terminated)
      /// @param ctx The COLTContext to use to store types and expressions
      ASTMaker(StringView strv, COLTContext& ctx) noexcept;
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

      /// @brief Obtains a view over the result of the parsing
      /// @return Vector of expressions representing the abstract syntax tree
      ContiguousView<PTR<Expr>> get_result() const noexcept { return expressions.to_view(); }
      /// @brief Steals the result of the parsing
      /// @return Vector of expressions representing the abstract syntax tree
      Vector<PTR<Expr>>&& steal_result() noexcept { return std::move(expressions); }

      /// @brief Check if the abstract syntax tree does not contain any expression
      /// @return True if the abstract syntax tree is empty
      bool is_empty() const noexcept { return expressions.get_size() == 0; }

    private:
      /// @brief Updates 'current_tkn' to the next token
      void consume_current_tkn() noexcept;

      /************* EXPRESSION PARSING ************/

      template<typename... Args>
      /// @brief Parses any Expr enclosed in parenthesis.
      /// Usage Example: parse_parenthesis(&AST::parse_unary)
      /// @param method_ptr The method pointer to parse inside the parenthesis
      PTR<Expr> parse_parenthesis(PTR<Expr>(ASTMaker::*method_ptr)(Args...), Args&&... args) noexcept;

      /// @brief Parses a LiteralExpr, VarReadExpr, FnCallExpr, or a UnaryExpr.
      PTR<Expr> parse_primary() noexcept;

      /// @brief Parses a BinaryExpr, or a 'primary_expr'.
      /// Expects a primary_expr.
      PTR<Expr> parse_binary(u8 precedence = 0) noexcept;

      /// @brief Parses a UnaryExpr.
      /// Precondition: current_tkn contains a UnaryOperator
      PTR<Expr> parse_unary() noexcept;

      PTR<Expr> parse_global_declaration() noexcept;

      PTR<Expr> parse_fn_decl() noexcept;

      PTR<Expr> parse_scope(bool one_expr = true) noexcept;

      PTR<Expr> parse_statement() noexcept;

      PTR<Expr> parse_condition() noexcept;

      PTR<Expr> parse_variable_decl(bool is_global) noexcept;
      
      PTR<Expr> parse_assignment(PTR<Expr> lhs) noexcept;
      
      PTR<const Type> parse_typename() noexcept;

      /************* PEEKING HELPERS ************/

      bool is_valid_scope_begin() const noexcept { return current_tkn == TKN_COLON; }

      /************* ERROR HANDLING HELPERS ************/

      template<typename... Args>
      /// @brief Validates that the current token is 'expected' and consumes it, else generates 'error'
      /// @tparam ...Args The parameter pack to format
      /// @param expected The expected token
      /// @param fmt The error format to print if the current token does not match 'expected'
      /// @param ...args The argument pack to format
      /// @return True if the token was invalid
      bool check_and_consume(Token expected, fmt::format_string<Args...> fmt, Args&&... args) noexcept;

      /// @brief Consumes tokens until a 'TKN_EOF' or 'TKN_SEMICOLON' is hit
      void panic_consume() noexcept;

      /// @brief Consumes tokens until a 'TKN_EOF' or 'TKN_LEFT_PAREN' is hit
      void panic_consume_rparen() noexcept;

      template<typename... Args>
      /// @brief Generates an error over the current lexeme
      /// @tparam ...Args The parameter pack to format
      /// @param fmt The format string
      /// @param ...args The argument pack to format
      void gen_error_lexeme(fmt::format_string<Args...> fmt, Args&& ...args) noexcept;

      template<typename... Args>
      /// @brief Generates an error over the current expression
      /// @tparam ...Args The parameter pack to format
      /// @param fmt The format string
      /// @param ...args The argument pack to format
      void gen_error_expr(fmt::format_string<Args...> fmt, Args&& ...args) noexcept;
    };
    
    template<typename ...Args>
    PTR<Expr> ASTMaker::parse_parenthesis(PTR<Expr>(ASTMaker::*method_ptr)(Args...), Args&&... args) noexcept
    {
      check_and_consume(TKN_LEFT_PAREN, "Expected a '('!");
      auto to_ret = (*this.*method_ptr)(std::forward<Args>(args)...); //Call method
      check_and_consume(TKN_RIGHT_PAREN, "Expected a ')'!");

      return to_ret;
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
        gen_error_lexeme(fmt, std::forward<Args>(args)...);
        return true;
      }
    }

    template<typename ...Args>
    void ASTMaker::gen_error_lexeme(fmt::format_string<Args...> fmt, Args&&... args) noexcept
    {
      auto line_info = lexer.get_line_info();
      GenerateError(line_info.line_nb, line_info.line_strv, lexer.get_current_lexeme(), fmt, std::forward<Args>(args)...);
      
      ++error_count;
      panic_consume();
    }
    
    template<typename ...Args>
    void ASTMaker::gen_error_expr(fmt::format_string<Args...> fmt, Args&&... args) noexcept
    {
      auto new_line_info = lexer.get_line_info();
      StringView line_strv = { current_lexeme_info.line_strv.get_data(),
        new_line_info.line_strv.get_data() + new_line_info.line_strv.get_size() };

      GenerateError(current_lexeme_info.line_nb, line_strv, current_lexeme_info.expression, fmt, std::forward<Args>(args)...);
      
      ++error_count;
      panic_consume();
    }
  }

  /// @brief An abstract tree of a COLT program
  struct AST
  {
    /// @brief The array of expressions
    Vector<PTR<Expr>> expressions;
    /// @brief The context storing type and expression informations
    COLTContext& ctx;

    /// @brief Creates an AST
    /// @param exprs The vector of expressions
    /// @param ctx The context storing the expressions
    AST(Vector<PTR<Expr>>&& exprs, COLTContext& ctx) noexcept
      : expressions(std::move(exprs)), ctx(ctx) {}
   };

  /// @brief Creates an Abstract Syntax Tree by parsing a StringView
  /// @param from The StringView to parse
  /// @param ctx The COLTContext where to store the expressions
  /// @return None if error where detected else the AST
  Optional<AST> CreateAST(StringView from, COLTContext& ctx) noexcept;
}

#endif //!HG_COLT_AST