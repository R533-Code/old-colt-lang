/** @file colt_expr.h
* Contains the representation of all expression of the Colt language.
*/

#ifndef HG_COLT_EXPR
#define HG_COLT_EXPR

#include <util/colt_pch.h>
#include <type/colt_type.h>
#include <ast/colt_operators.h>

namespace colt::lang
{
  //Forward declaration
  class COLTContext;

  /// @brief Abstract base class of all expressions
  class Expr
  {
  public:
    /// @brief The ID of an Expr, which allows to cast to the real type
    enum ExprID
      : u8
    {
      /// @brief Expr
      EXPR_BASE,
      /// @brief LiteralExpr
      EXPR_LITERAL,
      /// @brief UnaryExpr
      EXPR_UNARY,
      /// @brief BinaryExpr
      EXPR_BINARY,
      /// @brief ConvertExpr
      EXPR_CONVERT,
      /// @brief VarDeclExpr
      EXPR_VAR_DECL,
      /// @brief VarReadExpr
      EXPR_VAR_READ,
      /// @brief VarWriteExpr
      EXPR_VAR_WRITE,
      /// @brief FnDefExpr
      EXPR_FN_DEF,
      /// @brief FnCallExpr
      EXPR_FN_CALL,
      /// @brief FnReturnExpr
      EXPR_FN_RETURN,
      /// @brief ScopeExpr
      EXPR_SCOPE,
      /// @brief ConditionExpr
      EXPR_CONDITION,
    };

    /// @brief Helper for dyn_cast and is_a
    static constexpr ExprID classof_v = EXPR_BASE;		

  private:

    /// @brief The ID of the expression
    ExprID ID;

  public:
    Expr() = delete;
    Expr(const Expr&) = delete;
    Expr(Expr&&) = delete;

    /// @brief Constructor
    /// @param type The type of the expression	
    Expr(ExprID ID) noexcept
      : ID(ID) {}
    
    /// @brief Destructor
    virtual ~Expr() noexcept = default;

    /// @brief Returns the actual type of the Expr
    /// @return The ExprID of the current expression
    constexpr ExprID classof() const noexcept { return ID; }

    friend bool operator==(const Expr& lhs, const Expr& rhs) noexcept;
  };

  /// @brief Compares 2 non-null UniquePtr of Expr.
  /// Precondition: lhs && rhs
  /// @param lhs The left hand side
  /// @param rhs The right hand side
  /// @return True if equal
  bool operator==(const UniquePtr<Expr>& lhs, const UniquePtr<Expr>& rhs) noexcept;	

  /// @brief Represents a literal expression
  class LiteralExpr
    final : public Expr
  {
  public:
    /// @brief Helper for dyn_cast and is_a
    static constexpr ExprID classof_v = EXPR_LITERAL;

  private:

    /// @brief The literal value of the expression
    QWORD value;

  public:
    /// @brief No default constructor
    LiteralExpr() = delete;
    /// @brief Destructor
    ~LiteralExpr() noexcept override = default;

    /// @brief Constructor
    /// @param value The value of the literal expression
    LiteralExpr(QWORD value) noexcept
      : Expr(EXPR_LITERAL), value(value) {}

    /// @brief Returns the value of the literal expression
    /// @return QWORD representing the value
    QWORD get_value() const noexcept { return value; }

    /// @brief Creates a LiteralExpr
    /// @param value The value of the LiteralExpr
    /// @param ctx The COLTContext to store the resulting expression
    /// @return Pointer to the created expression
    static PTR<Expr> CreateExpr(QWORD value, COLTContext& ctx) noexcept;
  };

  /// @brief Represents a unary operation applied on an expression
  class UnaryExpr
    final : public Expr
  {
  public:
    /// @brief Helper for dyn_cast and is_a
    static constexpr ExprID classof_v = EXPR_UNARY;

  private:
    /// @brief The unary operator
    UnaryOperator operation;
    /// @brief The expression on which the operator is applied
    PTR<Expr> child;

    /// @brief Constructor
    /// @param tkn_op The unary operator of the expression
    /// @param child The expression on which the operator is applied
    /// @param is_post For TKN_PLUS_PLUS/TKN_MINUS_MINUS specifies if the operator is pre/post
    UnaryExpr(Token tkn_op, PTR<Expr> child, bool is_post = false) noexcept
      : Expr(EXPR_UNARY), operation(TokenToUnaryOperator(tkn_op, is_post)), child(child) {}
  
  public:
    //No default copy constructor 
    UnaryExpr(const UnaryExpr&) = delete;
    //No default constructor
    UnaryExpr() = delete;
    /// @brief Destructor
    ~UnaryExpr() noexcept override = default;


    /// @brief Returns the child of the unary expression
    /// @return Pointer to the child
    PTR<const Expr> get_child() const noexcept { return child; }
    
    /// @brief Returns the operation that should be performed by the UnaryExpr
    /// @return The operation
    UnaryOperator get_operation() const noexcept { return operation; }

    /// @brief Creates a UnaryExpr
    /// @param tkn_op The unary operator of the expression
    /// @param child The expression on which the operator is applied
    /// @param ctx The COLTContext to store the resulting expression
    /// @return Pointer to the expression
    static PTR<Expr> CreateExpr(Token tkn, PTR<Expr> child, COLTContext& ctx) noexcept;
    /// @brief Creates a UnaryExpr
    /// @param tkn_op The unary operator of the expression
    /// @param is_post For TKN_PLUS_PLUS/TKN_MINUS_MINUS specifies if the operator is pre/post
    /// @param child The expression on which the operator is applied
    /// @param ctx The COLTContext to store the resulting expression
    /// @return Pointer to the expression
    static PTR<Expr> UnaryExpr::CreateExpr(Token tkn, bool is_post, PTR<Expr> child, COLTContext& ctx) noexcept;
  };

  /// @brief Represents a binary operation applied on two expressions
  class BinaryExpr
    final : public Expr
  {
  public:
    /// @brief Helper for dyn_cast and is_a
    static constexpr ExprID classof_v = EXPR_BINARY;

  private:
    /// @brief Left-hand side of the expression
    PTR<Expr> lhs;
    /// @brief The binary operator
    BinaryOperator operation;
    /// @brief Right-hand side of the expression
    PTR<Expr> rhs;

    /// @brief Creates a binary expression of 'lhs' 'operation' 'rhs'
    /// @param lhs The left hand side of the expression
    /// @param operation The binary operator token
    /// @param rhs The right hand side of the expression
    BinaryExpr(PTR<Expr> lhs, Token operation, PTR<Expr> rhs) noexcept
      : Expr(EXPR_BINARY), lhs(lhs), operation(TokenToBinaryOperator(operation)), rhs(rhs) {}

  public:
    //No default copy constructor
    BinaryExpr(const BinaryExpr&) = delete;
    //No default constructor
    BinaryExpr() = delete;
    /// @brief Destructor
    ~BinaryExpr() noexcept override = default;		

    /// @brief Returns the left hand side of the unary expression
    /// @return Pointer to the lhs
    PTR<const Expr> get_LHS() const noexcept { return lhs; }

    /// @brief Returns the right hand side of the unary expression
    /// @return Pointer to the rhs
    PTR<const Expr> get_RHS() const noexcept { return rhs; }

    /// @brief Returns the operation that should be performed by the UnaryExpr
    /// @return The operation
    BinaryOperator get_operation() const noexcept { return operation; }

    /// @brief Creates a BinaryExpr
    /// @param lhs The left hand side of the expression
    /// @param operation The binary operator token
    /// @param rhs The right hand side of the expression
    /// @param ctx The COLTContext to store the resulting expression
    /// @return Pointer to the created expression
    static PTR<Expr> CreateExpr(PTR<Expr> lhs, Token op, PTR<Expr> rhs, COLTContext& ctx) noexcept;
  };

  /// @brief Represents a conversion applied to an expression
  class ConvertExpr
    final : public Expr
  {
  public:
    /// @brief Helper for dyn_cast and is_a
    static constexpr ExprID classof_v = EXPR_CONVERT;

  private:
    /// @brief The expression to convert
    PTR<Expr> to_convert;

    /// @brief Constructor
    /// @param to_convert The expression to convert
    /// @param convert_to The type to convert to
    ConvertExpr(PTR<Expr> to_convert) noexcept
      : Expr(EXPR_CONVERT), to_convert(to_convert) {}
  
  public:
    //No default copy constructor 
    ConvertExpr(const ConvertExpr&) = delete;
    //No default constructor
    /// @brief No default constructor
    ConvertExpr() = delete;
    /// @brief Destructor
    ~ConvertExpr() noexcept override = default;

    /// @brief Get the expression to convert
    /// @return The expression to converse
    PTR<const Expr> get_child() const noexcept { return to_convert; }

    /// @brief Creates a ConvertExpr
    /// @param to_convert The expression to convert
    /// @param ctx The COLTContext to store the resulting expression
    /// @return Pointer to the created expression
    static PTR<Expr> CreateExpr(PTR<Expr> to_convert, COLTContext& ctx) noexcept;
  };

  /// @brief Represents a declaration of a variable
  class VarDeclExpr
    final : public Expr
  {
  public:
    /// @brief Helper for dyn_cast and is_a
    static constexpr ExprID classof_v = EXPR_VAR_DECL;

  private:
    /// @brief True if the variable is global
    bool is_global_v;
    /// @brief The initial value of the variable, can be null
    PTR<Expr> init_value;
    /// @brief The name of the variable
    StringView name;

    /// @brief Constructs a variable declaration expression
    /// @param name The name of the variable
    /// @param init_value The initial value of the variable, can be null
    /// @param is_global True if the variable is global
    VarDeclExpr(StringView name, PTR<Expr> init_value, bool is_global) noexcept
      : Expr(EXPR_VAR_DECL), is_global_v(is_global), init_value(init_value), name(name) {}
  
  public:
    //No default copy constructor 
    VarDeclExpr(const VarDeclExpr&) = delete;
    //No default constructor
    VarDeclExpr() = delete;
    /// @brief Destructor
    ~VarDeclExpr() noexcept override = default;

    /// @brief Get the expression to convert
    /// @return The expression to converse
    PTR<const Expr> get_value() const noexcept { return init_value; }

    /// @brief Returns the name of the global variable
    /// @return The name of the variable
    StringView get_name() const noexcept { return name; }

    /// @brief Check if the variable is global or not
    /// @return True if the variable is global
    bool is_global() const noexcept { return is_global_v; }

    /// @brief Check if the variable is initialized
    /// @return True if initialized
    bool is_initialized() const noexcept { return init_value != nullptr; }

    /// @brief Creates a VarDeclExpr
    /// @param name The name of the variable
    /// @param init_value The value of the variable, which can be null
    /// @param is_global True if the variable is global
    /// @param ctx The COLTContext to store the resulting expression
    /// @return Pointer to the created expression
    static PTR<Expr> CreateExpr(StringView name, PTR<Expr> init_value, bool is_global, COLTContext& ctx) noexcept;
  };

  /// @brief Represents a read from a variable
  class VarReadExpr
    final : public Expr
  {
  public:
    /// @brief Helper for dyn_cast and is_a
    static constexpr ExprID classof_v = EXPR_VAR_READ;

  private:
    /// @brief True if the variable is global
    u64 local_ID;
    /// @brief The name of the variable
    StringView name;
    
    /// @brief Constructs a read from a global variable of name 'name'
    /// @param name The name of the variable
    explicit VarReadExpr(StringView name) noexcept
      : Expr(EXPR_VAR_READ), local_ID(std::numeric_limits<u64>::max()), name(name) {}

    /// @brief Constructs a read from a local variable of name 'name'
    /// @param name The name of the variable
    /// @param local_ID The local ID of the variable
    VarReadExpr(StringView name, u64 local_ID) noexcept
      : Expr(EXPR_VAR_READ), local_ID(local_ID), name(name) { assert_true(!is_global(), "Invalid local ID!"); }

  public:
    //No default copy constructor 
    VarReadExpr(const VarReadExpr&) = delete;
    //No default constructor
    VarReadExpr() = delete;
    /// @brief Destructor
    ~VarReadExpr() noexcept override = default;

    /// @brief Returns the name of the global variable
    /// @return The name of the variable
    StringView get_name() const noexcept { return name; }

    /// @brief Check if the variable is global
    /// @return True if the variable is global
    bool is_global() const noexcept { return local_ID == std::numeric_limits<u64>::max(); }

    /// @brief Returns the local ID.
    /// The variable should not be global.
    /// @return The local ID
    u64 get_local_ID() const noexcept { assert_true(!is_global(), "Variable was global!"); return local_ID; }

    /// @brief Returns the local ID, but does not assert if the variable is global.
    /// @return The local ID
    u64 unsafe_get_local_id() const noexcept { return local_ID; }

    /// @brief Creates a VarReadExpr
    /// @param name The name of the variable
    /// @param ID The local ID of the variable
    /// @param ctx The COLTContext to store the resulting expression
    /// @return Pointer to the created expression
    static PTR<Expr> CreateExpr(StringView name, u64 ID, COLTContext& ctx) noexcept;
    /// @brief Creates a VarReadExpr of a global variables
    /// @param name The name of the variable
    /// @param ctx The COLTContext to store the resulting expression
    /// @return Pointer to the created expression
    static PTR<Expr> CreateExpr(StringView name, COLTContext& ctx) noexcept;
  };

  /// @brief Represents a write to a variable
  class VarWriteExpr
    final : public Expr
  {
  public:
    /// @brief Helper for dyn_cast and is_a
    static constexpr ExprID classof_v = EXPR_VAR_WRITE;

  private:
    /// @brief True if the variable is global
    u64 local_ID;
    /// @brief The value of the variable at initialization
    PTR<Expr> value;
    /// @brief The name of the variable
    StringView name;

    /// @brief Constructs a write to a global variable
    /// @param name The name of the variable to write to
    /// @param value The value to write to the variable
    VarWriteExpr(StringView name, PTR<Expr> value) noexcept
      : Expr(EXPR_VAR_WRITE), local_ID(std::numeric_limits<u64>::max()), value(value), name(name) {}

    /// @brief Constructs a write to a local variable
    /// @param name The name of the variable to write to
    /// @param value The value to write to the variable
    /// @param local_ID The local ID of the variable
    VarWriteExpr(StringView name, PTR<Expr> value, u64 local_ID) noexcept
      : Expr(EXPR_VAR_WRITE), local_ID(local_ID), value(value), name(name) { assert_true(!is_global(), "Invalid local ID!"); }

  public:
    //No default copy constructor 
    VarWriteExpr(const VarWriteExpr&) = delete;
    //No default constructor
    VarWriteExpr() = delete;
    /// @brief Destructor
    ~VarWriteExpr() noexcept override = default;

    /// @brief Get the expression to convert
    /// @return The expression to converse
    PTR<const Expr> get_value() const noexcept { return value; }

    /// @brief Returns the name of the global variable
    /// @return The name of the variable
    StringView get_name() const noexcept { return name; }

    /// @brief Check if the variable is global
    /// @return True if the variable is global
    bool is_global() const noexcept { return local_ID == std::numeric_limits<u64>::max(); }

    /// @brief Returns the local ID.
    /// The variable should not be global.
    /// @return The local ID
    u64 get_local_ID() const noexcept { assert_true(!is_global(), "Variable was global!"); return local_ID; }

    /// @brief Returns the local ID.
    /// @return The local ID
    u64 unsafe_get_local_id() const noexcept { return local_ID; }

    /// @brief Creates a VarWriteExpr
    /// @param name The name of the variable
    /// @param value The value to write to the variable
    /// @param ID The local ID of the variable
    /// @param ctx The COLTContext to store the resulting expression
    /// @return Pointer to the created expression
    static PTR<Expr> CreateExpr(StringView name, PTR<Expr> value, u64 ID, COLTContext& ctx) noexcept;
    /// @brief Creates a VarWriteExpr to a global variable
    /// @param name The name of the variable
    /// @param value The value to write to the variable
    /// @param ctx The COLTContext to store the resulting expression
    /// @return Pointer to the created expression
    static PTR<Expr> CreateExpr(StringView name, PTR<Expr> value, COLTContext& ctx) noexcept;
  };

  class FnReturnExpr
    final : public Expr
  {
  public:
    /// @brief Helper for dyn_cast and is_a
    static constexpr ExprID classof_v = EXPR_FN_RETURN;

  private:
    /// @brief The value to return from the function (can be NULL)
    PTR<Expr> to_ret;

    /// @brief Constructs a function return
    /// @param to_ret The value to return, can be null
    FnReturnExpr(PTR<Expr> to_ret) noexcept
      : Expr(EXPR_FN_RETURN), to_ret(to_ret) {}
  
  public:
    //No default copy constructor 
    FnReturnExpr(const FnReturnExpr&) = delete;
    //No default constructor
    FnReturnExpr() = delete;
    /// @brief Destructor
    ~FnReturnExpr() noexcept override = default;

    /// @brief Get the return value
    /// @return The value
    PTR<const Expr> get_value() const noexcept { return to_ret; }

    /// @brief Creates a FnReturnExpr
    /// @param to_ret The value to return, can be null
    /// @param ctx The COLTContext to store the resulting expression
    /// @return Pointer to the created expression
    static PTR<Expr> CreateExpr(PTR<Expr> to_ret, COLTContext& ctx) noexcept;
  };

  /// @brief Represents a function definition
  class FnDefExpr
    final : public Expr
  {
  public:
    /// @brief Helper for dyn_cast and is_a
    static constexpr ExprID classof_v = EXPR_FN_DEF;

  private:
    /// @brief The body of the function
    PTR<Expr> body;
    /// @brief The argument of the function
    SmallVector<StringView, 4> arguments_name;
    /// @brief Contains list of pointer to the registered returns of the function.
    /// Can contain null.
    SmallVector<FnReturnExpr*, 4> return_list;
    /// @brief The name of the function
    StringView name;

    /// @brief Creates function definition
    /// @param name The name of the function
    /// @param arguments_name The arguments name
    /// @param body The body of the function
    FnDefExpr(StringView name, SmallVector<StringView, 4>&& arguments_name, PTR<Expr> body = nullptr) noexcept
      : Expr(EXPR_FN_DEF), body(body), arguments_name(std::move(arguments_name)), name(name) { /*assert_true(Expr::getType()->isFnPtr(), "Expected a function type!");*/
    }

  public:
    //No default copy constructor 
    FnDefExpr(const FnDefExpr&) = delete;
    //No default constructor
    FnDefExpr() = delete;
    /// @brief Destructor
    ~FnDefExpr() noexcept override = default;

    /// @brief Sets the body of the function to 'body_p'
    /// @param body_p The new body of the function
    void set_body(PTR<Expr> body_p) noexcept { body = body_p; }

    /// @brief Get the expression to convert
    /// @return The expression to converse
    PTR<const Expr> get_body() const noexcept { return body; }

    /// @brief Pushes a return to the list of return of the function
    /// @param ret The pointer to the FnReturnExpr
    void push_return(FnReturnExpr* ret) noexcept { return_list.push_back(ret); }

    /// @brief Returns a const reference over the list of returns
    /// @return Const reference of the list of returns
    ContiguousView<FnReturnExpr*> get_return_list() const noexcept { return return_list.to_view(); }

    /// @brief Returns the name of the global variable
    /// @return The name of the variable
    StringView get_name() const noexcept { return name; }

    /// @brief Returns the parameter names
    /// @return View over the parameter names
    ContiguousView<StringView> get_params_name() const noexcept { return arguments_name.to_view(); }

    /// @brief Creates a FnDefExpr
    /// @param name The name of the function
    /// @param arguments_name The arguments name
    /// @param ctx The COLTContext to store the resulting expression
    /// @return Pointer to the created expression
    static PTR<Expr> CreateExpr(StringView name, SmallVector<StringView, 4>&& arguments_name, COLTContext& ctx) noexcept;
    /// @brief Creates a FnDefExpr
    /// @param name The name of the function
    /// @param arguments_name The arguments name
    /// @param body The body of the function
    /// @param ctx The COLTContext to store the resulting expression
    /// @return Pointer to the created expression
    static PTR<Expr> CreateExpr(StringView name, SmallVector<StringView, 4>&& arguments_name, PTR<Expr> body, COLTContext& ctx) noexcept;
  };

  /// @brief Represents a scope
  class ScopeExpr
    final : public Expr
  {
  public:
    /// @brief Helper for dyn_cast and is_a
    static constexpr ExprID classof_v = EXPR_SCOPE;

  private:
    /// @brief The argument of the function
    Vector<PTR<Expr>> body_expr;

    /// @brief Constructs a ScopeExpr from an array of Expr*
    /// @param body_expr The Vector of expressions contained in the scope
    ScopeExpr(Vector<PTR<Expr>>&& body_expr = {}) noexcept
      : Expr(EXPR_SCOPE), body_expr(std::move(body_expr)) {}

  public:
    //No default copy constructor 
    ScopeExpr(const ScopeExpr&) = delete;
    //No default constructor
    ScopeExpr() = delete;
    /// @brief Destructor
    ~ScopeExpr() noexcept override = default;		

    /// @brief Sets the body of the scope to 'body'
    /// @param body The new body
    void set_body_array(Vector<PTR<Expr>>&& body) noexcept { body_expr = std::move(body); }

    /// @brief Get the expression to convert
    /// @return The expression to converse
    ContiguousView<PTR<Expr>> get_body_array() const noexcept { return body_expr.to_view(); }

    /// @brief Creates a ScopeExpr
    /// @param body The body of the ScopeExpr
    /// @param ctx The COLTContext to store the resulting expression
    /// @return Pointer to the created expression
    static PTR<Expr> CreateExpr(Vector<PTR<Expr>>&& body, COLTContext& ctx) noexcept;
  };

  /// @brief Represents a scope
  class ConditionExpr
    final : public Expr
  {
  public:
    /// @brief Helper for dyn_cast and is_a
    static constexpr ExprID classof_v = EXPR_CONDITION;

  private:
    /// @brief If condition
    PTR<Expr> if_cond;
    /// @brief If statement, which is the statement to execute if 'if_cond' evaluates to true
    PTR<Expr> if_stmt;
    /// @brief Else statement, can be null
    PTR<Expr> else_stmt;

    /// @brief Constructs a condition expression
    /// @param if_cond The if condition
    /// @param if_stmt The statement to evaluate if the if condition is true
    /// @param else_stmt The else statement, which can be null
    ConditionExpr(PTR<Expr> if_cond, PTR<Expr> if_stmt, PTR<Expr> else_stmt) noexcept
      : Expr(EXPR_CONDITION), if_cond(if_cond), if_stmt(if_stmt), else_stmt(else_stmt) {}
  
  public:
    //No default copy constructor 
    ConditionExpr(const ConditionExpr&) = delete;
    //No default constructor
    ConditionExpr() = delete;
    /// @brief Destructor
    ~ConditionExpr() noexcept override = default;

    /// @brief Get the expression to convert
    /// @return The expression to converse
    PTR<const Expr> get_if_condition() const noexcept { return if_cond; }

    /// @brief Get the expression to convert
    /// @return The expression to converse
    PTR<const Expr> get_if_statement() const noexcept { return if_stmt; }

    /// @brief Get the expression to convert
    /// @return The expression to converse
    PTR<const Expr> get_else_statement() const noexcept { return else_stmt; }

    /// @brief Creates a ConditionExpr
    /// @param if_cond The if condition
    /// @param if_stmt The statement to evaluate if the if condition is true
    /// @param else_stmt The else statement, which can be null
    /// @param ctx The COLTContext to store the resulting expression
    /// @return Pointer to the created expression
    static PTR<Expr> CreateExpr(PTR<Expr> if_cond, PTR<Expr> if_stmt, PTR<Expr> else_stmt, COLTContext& ctx) noexcept;
  };
}

namespace colt
{
  size_t hash(const lang::Expr& expr) noexcept;
}

#endif //!HG_COLT_EXPR