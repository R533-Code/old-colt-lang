/** @file colt_expr.h
* Contains the representation of all expression of the Colt language.
* The individual nodes of the abstract syntax tree are made of the expressions
* in this file.
*/

#ifndef HG_COLT_EXPR
#define HG_COLT_EXPR

#include <util/colt_pch.h>
#include <parsing/colt_error_report.h>
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
      /// @brief ErrorExpr
      EXPR_ERROR,
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
      /// @brief FnDeclExpr
      EXPR_FN_DECL,
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
      /// @brief ForLoopExpr
      EXPR_FOR_LOOP,
      /// @brief WhileLoopExpr
      EXPR_WHILE_LOOP,
      /// @brief BreakContinueExpr
      EXPR_BREAK_CONTINUE,
      /// @brief NoOpExpr
      EXPR_NOP
    };

    /// @brief Helper for dyn_cast and is_a
    static constexpr ExprID classof_v = EXPR_BASE;		

  private:

    /// @brief The ID of the expression
    ExprID ID;
    /// @brief The type of the expression
    PTR<const Type> type;
    /// @brief The source code information of the current expression
    SourceCodeExprInfo src_info;

  public:
    Expr() = delete;
    Expr(const Expr&) = delete;
    Expr(Expr&&) = delete;

    /// @brief Constructor
    /// @param ID The expression ID
    /// @param type The type of the expression	
    /// @param src_info The source code information
    Expr(ExprID ID, PTR<const Type> type, const SourceCodeExprInfo& src_info) noexcept
      : ID(ID), type(type), src_info(src_info) {}
    
    /// @brief Destructor
    virtual ~Expr() noexcept = default;

    /// @brief Returns the actual type of the Expr
    /// @return The ExprID of the current expression
    constexpr ExprID classof() const noexcept { return ID; }

    /// @brief Returns the type of the expression
    /// @return The type of the expression
    constexpr PTR<const Type> get_type() const noexcept { return type; }

    /// @brief Returns the source code information of the expressions
    /// @return The source code information of the expression
    constexpr const SourceCodeExprInfo& get_src_code() const noexcept { return src_info; }
  };

  /// @brief Represents a literal expression
  class ErrorExpr
    final : public Expr
  {
  public:
    /// @brief Helper for dyn_cast and is_a
    static constexpr ExprID classof_v = EXPR_ERROR;

    /// @brief No default constructor
    ErrorExpr(PTR<const Type> type)
      : Expr(EXPR_ERROR, type, {}) {}
    /// @brief Destructor
    ~ErrorExpr() noexcept override = default;

    /// @brief Creates an ErrorExpr
    /// @param ctx The COLTContext to store the resulting expression
    /// @return Pointer to the created expression
    static PTR<Expr> CreateExpr(COLTContext& ctx) noexcept;
  };

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
    /// @param type The type of the resulting expression
    /// @param src_info The source code information
    LiteralExpr(QWORD value, PTR<const Type> type, const SourceCodeExprInfo& src_info) noexcept
      : Expr(EXPR_LITERAL, type, src_info), value(value)
    {
      assert(type->is_builtin());
    }

    /// @brief Returns the value of the literal expression
    /// @return QWORD representing the value
    QWORD get_value() const noexcept { return value; }

    /// @brief Creates a LiteralExpr
    /// @param value The value of the LiteralExpr
    /// @param type The type of the resulting expression
    /// @param src_info The source code information
    /// @param ctx The COLTContext to store the resulting expression
    /// @return Pointer to the created expression
    static PTR<Expr> CreateExpr(QWORD value, PTR<const Type> type, const SourceCodeExprInfo& src_info, COLTContext& ctx) noexcept;
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
  
  public:
    //No default copy constructor
    UnaryExpr(const UnaryExpr&) = delete;
    //No default constructor
    UnaryExpr() = delete;
    /// @brief Destructor
    ~UnaryExpr() noexcept override = default;
    /// @brief Constructor
    /// @param type The type of the resulting expression
    /// @param tkn_op The unary operator of the expression
    /// @param child The expression on which the operator is applied
    /// @param is_post For TKN_PLUS_PLUS/TKN_MINUS_MINUS specifies if the operator is pre/post
    /// @param src_info The source code information
    UnaryExpr(PTR<const Type> type, Token tkn_op, PTR<Expr> child, bool is_post, const SourceCodeExprInfo& src_info) noexcept
      : Expr(EXPR_UNARY, type, src_info), operation(TokenToUnaryOperator(tkn_op, is_post)), child(child) {}


    /// @brief Returns the child of the unary expression
    /// @return Pointer to the child
    PTR<const Expr> get_child() const noexcept { return child; }
    
    /// @brief Returns the operation that should be performed by the UnaryExpr
    /// @return The operation
    UnaryOperator get_operation() const noexcept { return operation; }

    /// @brief Creates a UnaryExpr
    /// @param type The type of the resulting expression
    /// @param tkn The unary operator of the expression
    /// @param child The expression on which the operator is applied
    /// @param src_info The source code information
    /// @param ctx The COLTContext to store the resulting expression
    /// @return Pointer to the expression
    static PTR<Expr> CreateExpr(PTR<const Type> type, Token tkn, PTR<Expr> child, const SourceCodeExprInfo& src_info, COLTContext& ctx) noexcept;
    /// @brief Creates a UnaryExpr
    /// @param type The type of the resulting expression
    /// @param tkn The unary operator of the expression
    /// @param is_post For TKN_PLUS_PLUS/TKN_MINUS_MINUS specifies if the operator is pre/post
    /// @param child The expression on which the operator is applied
    /// @param src_info The source code information
    /// @param ctx The COLTContext to store the resulting expression
    /// @return Pointer to the expression
    static PTR<Expr> CreateExpr(PTR<const Type> type, Token tkn, bool is_post, PTR<Expr> child, const SourceCodeExprInfo& src_info, COLTContext& ctx) noexcept;
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

  public:
    //No default copy constructor
    BinaryExpr(const BinaryExpr&) = delete;
    //No default constructor
    BinaryExpr() = delete;
    /// @brief Destructor
    ~BinaryExpr() noexcept override = default;		
    /// @brief Creates a binary expression of 'lhs' 'operation' 'rhs'
    /// @param type The type of the expression
    /// @param lhs The left hand side of the expression
    /// @param operation The binary operator token
    /// @param rhs The right hand side of the expression
    /// @param src_info The source code information
    BinaryExpr(PTR<const Type> type, PTR<Expr> lhs, Token operation, PTR<Expr> rhs, const SourceCodeExprInfo& src_info) noexcept
      : Expr(EXPR_BINARY, type, src_info), lhs(lhs), operation(TokenToBinaryOperator(operation)), rhs(rhs) {}

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
    /// @param type The type of the resulting expression
    /// @param lhs The left hand side of the expression
    /// @param op The binary operator token
    /// @param rhs The right hand side of the expression
    /// @param src_info The source code information
    /// @param ctx The COLTContext to store the resulting expression
    /// @return Pointer to the created expression
    static PTR<Expr> CreateExpr(PTR<const Type> type, PTR<Expr> lhs, Token op, PTR<Expr> rhs, const SourceCodeExprInfo& src_info, COLTContext& ctx) noexcept;
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

  public:
    //No default copy constructor 
    ConvertExpr(const ConvertExpr&) = delete;
    //No default constructor
    /// @brief No default constructor
    ConvertExpr() = delete;
    /// @brief Destructor
    ~ConvertExpr() noexcept override = default;
    /// @brief Constructor
    /// @param type The new type of the expression
    /// @param to_convert The expression to convert
    /// @param src_info The source code information
    ConvertExpr(PTR<const Type> type, PTR<Expr> to_convert, const SourceCodeExprInfo& src_info) noexcept
      : Expr(EXPR_CONVERT, type, src_info), to_convert(to_convert) {}

    /// @brief Get the expression to convert
    /// @return The expression to converse
    PTR<const Expr> get_child() const noexcept { return to_convert; }

    /// @brief Creates a ConvertExpr
    /// @param type The type of the resulting expression
    /// @param to_convert The expression to convert
    /// @param src_info The source code information
    /// @param ctx The COLTContext to store the resulting expression
    /// @return Pointer to the created expression
    static PTR<Expr> CreateExpr(PTR<const Type> type, PTR<Expr> to_convert, const SourceCodeExprInfo& src_info, COLTContext& ctx) noexcept;
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
  
  public:
    //No default copy constructor 
    VarDeclExpr(const VarDeclExpr&) = delete;
    //No default constructor
    VarDeclExpr() = delete;
    /// @brief Destructor
    ~VarDeclExpr() noexcept override = default;
    /// @brief Constructs a variable declaration expression
    /// @param type The type of the resulting expression
    /// @param name The name of the variable
    /// @param init_value The initial value of the variable, can be null
    /// @param is_global True if the variable is global
    /// @param src_info The source code information
    VarDeclExpr(PTR<const Type> type, StringView name, PTR<Expr> init_value, bool is_global, const SourceCodeExprInfo& src_info) noexcept
      : Expr(EXPR_VAR_DECL, type, src_info), is_global_v(is_global), init_value(init_value), name(name) {}

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
    /// @param type The type of the resulting expression
    /// @param name The name of the variable
    /// @param init_value The value of the variable, which can be null
    /// @param is_global True if the variable is global
    /// @param src_info The source code information
    /// @param ctx The COLTContext to store the resulting expression
    /// @return Pointer to the created expression
    static PTR<Expr> CreateExpr(PTR<const Type> type, StringView name, PTR<Expr> init_value, bool is_global, const SourceCodeExprInfo& src_info, COLTContext& ctx) noexcept;
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

  public:
    //No default copy constructor 
    VarReadExpr(const VarReadExpr&) = delete;
    //No default constructor
    VarReadExpr() = delete;
    /// @brief Destructor
    ~VarReadExpr() noexcept override = default;
    /// @brief Constructs a read from a global variable of name 'name'
    /// @param type The type of the resulting expression
    /// @param name The name of the variable
    /// @param src_info The source code information
    VarReadExpr(PTR<const Type> type, StringView name, const SourceCodeExprInfo& src_info) noexcept
      : Expr(EXPR_VAR_READ, type, src_info), local_ID(std::numeric_limits<u64>::max()), name(name) {}
    /// @brief Constructs a read from a local variable of name 'name'
    /// @param type The type of the resulting expression
    /// @param name The name of the variable
    /// @param local_ID The local ID of the variable
    /// @param src_info The source code information
    VarReadExpr(PTR<const Type> type, StringView name, u64 local_ID, const SourceCodeExprInfo& src_info) noexcept
      : Expr(EXPR_VAR_READ, type, src_info), local_ID(local_ID), name(name) { assert_true(!is_global(), "Invalid local ID!"); }

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
    /// @param type The type of the resulting expression
    /// @param name The name of the variable
    /// @param ID The local ID of the variable
    /// @param src_info The source code information
    /// @param ctx The COLTContext to store the resulting expression
    /// @return Pointer to the created expression
    static PTR<Expr> CreateExpr(PTR<const Type> type, StringView name, u64 ID, const SourceCodeExprInfo& src_info, COLTContext& ctx) noexcept;
    /// @brief Creates a VarReadExpr of a global variables
    /// @param type The type of the resulting expression
    /// @param name The name of the variable
    /// @param src_info The source code information
    /// @param ctx The COLTContext to store the resulting expression
    /// @return Pointer to the created expression
    static PTR<Expr> CreateExpr(PTR<const Type> type, StringView name, const SourceCodeExprInfo& src_info, COLTContext& ctx) noexcept;
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

  public:
    //No default copy constructor 
    VarWriteExpr(const VarWriteExpr&) = delete;
    //No default constructor
    VarWriteExpr() = delete;
    /// @brief Destructor
    ~VarWriteExpr() noexcept override = default;
    /// @brief Constructs a write to a global variable
    /// @param type The type of the resulting expression
    /// @param name The name of the variable to write to
    /// @param value The value to write to the variable
    /// @param src_info The source code information
    VarWriteExpr(PTR<const Type> type, StringView name, PTR<Expr> value, const SourceCodeExprInfo& src_info) noexcept
      : Expr(EXPR_VAR_WRITE, type, src_info), local_ID(std::numeric_limits<u64>::max()), value(value), name(name) {}
    /// @brief Constructs a write to a local variable
    /// @param type The type of the resulting expression
    /// @param name The name of the variable to write to
    /// @param value The value to write to the variable
    /// @param local_ID The local ID of the variable
    /// @param src_info The source code information
    VarWriteExpr(PTR<const Type> type, StringView name, PTR<Expr> value, u64 local_ID, const SourceCodeExprInfo& src_info) noexcept
      : Expr(EXPR_VAR_WRITE, type, src_info), local_ID(local_ID), value(value), name(name) { assert_true(!is_global(), "Invalid local ID!"); }

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
    /// @param type The type of the resulting expression
    /// @param name The name of the variable
    /// @param value The value to write to the variable
    /// @param ID The local ID of the variable
    /// @param src_info The source code information
    /// @param ctx The COLTContext to store the resulting expression
    /// @return Pointer to the created expression
    static PTR<Expr> CreateExpr(PTR<const Type> type, StringView name, PTR<Expr> value, u64 ID, const SourceCodeExprInfo& src_info, COLTContext& ctx) noexcept;
    /// @brief Creates a VarWriteExpr to a global variable
    /// @param type The type of the resulting expression
    /// @param name The name of the variable
    /// @param value The value to write to the variable
    /// @param src_info The source code information
    /// @param ctx The COLTContext to store the resulting expression
    /// @return Pointer to the created expression
    static PTR<Expr> CreateExpr(PTR<const Type> type, StringView name, PTR<Expr> value, const SourceCodeExprInfo& src_info, COLTContext& ctx) noexcept;
  };

  /// @brief Return expression
  class FnReturnExpr
    final : public Expr
  {
  public:
    /// @brief Helper for dyn_cast and is_a
    static constexpr ExprID classof_v = EXPR_FN_RETURN;

  private:
    /// @brief The value to return from the function (can be NULL)
    PTR<Expr> to_ret;

  public:
    //No default copy constructor 
    FnReturnExpr(const FnReturnExpr&) = delete;
    //No default constructor
    FnReturnExpr() = delete;
    /// @brief Destructor
    ~FnReturnExpr() noexcept override = default;
    /// @brief Constructs a function return
    /// @param type The type of the resulting expression
    /// @param to_ret The value to return, can be null
    /// @param src_info The source code information
    FnReturnExpr(PTR<const Type> type, PTR<Expr> to_ret, const SourceCodeExprInfo& src_info) noexcept
      : Expr(EXPR_FN_RETURN, type, src_info), to_ret(to_ret) {}

    /// @brief Get the return value
    /// @return The value
    PTR<const Expr> get_value() const noexcept { return to_ret; }

    /// @brief Creates a FnReturnExpr
    /// @param to_ret The value to return, can be null
    /// @param src_info The source code information
    /// @param ctx The COLTContext to store the resulting expression
    /// @return Pointer to the created expression
    static PTR<Expr> CreateExpr(PTR<Expr> to_ret, const SourceCodeExprInfo& src_info, COLTContext& ctx) noexcept;
  };

  /// @brief Represents a function definition
  class FnDeclExpr
    final : public Expr
  {
  public:
    /// @brief Helper for dyn_cast and is_a
    static constexpr ExprID classof_v = EXPR_FN_DECL;

  private:
    /// @brief The argument of the function
    SmallVector<StringView, 4> arguments_name;
    /// @brief The name of the function
    StringView name;

  public:
    //No default copy constructor 
    FnDeclExpr(const FnDeclExpr&) = delete;
    //No default constructor
    FnDeclExpr() = delete;
    /// @brief Destructor
    ~FnDeclExpr() noexcept override = default;
    /// @brief Creates function definition
    /// @param type The function type
    /// @param name The name of the function
    /// @param arguments_name The arguments name
    /// @param src_info The source code information
    FnDeclExpr(PTR<const Type> type, StringView name, SmallVector<StringView, 4>&& arguments_name, const SourceCodeExprInfo& src_info) noexcept
      : Expr(EXPR_FN_DECL, type, src_info), arguments_name(std::move(arguments_name)), name(name)
    {
      assert_true(type->is_fn(), "Expected a function type!");
    }

    /// @brief Returns the name of the global variable
    /// @return The name of the variable
    StringView get_name() const noexcept { return name; }

    /// @brief Returns the count of parameter the function accepts
    /// @return The parameter count of the function
    size_t get_params_count() const noexcept { return arguments_name.get_size(); }
    /// @brief Returns the parameter names
    /// @return View over the parameter names
    ContiguousView<StringView> get_params_name() const noexcept { return arguments_name.to_view(); }
    /// @brief Returns the parameter types
    /// @return View over the parameter types
    ContiguousView<PTR<const Type>> get_params_type() const noexcept { return as<PTR<const FnType>>(get_type())->get_params_type(); }
    /// @brief Returns the return type of the function
    /// @return The return type of the function
    PTR<const Type> get_return_type() const noexcept { return static_cast<const FnType*>(get_type())->get_return_type(); }

    /// @brief Creates a FnDeclExpr
    /// @param type The type of the resulting expression
    /// @param name The name of the function
    /// @param arguments_name The arguments name
    /// @param src_info The source code information
    /// @param ctx The COLTContext to store the resulting expression
    /// @return Pointer to the created expression
    static PTR<Expr> CreateExpr(PTR<const Type> type, StringView name, SmallVector<StringView, 4>&& arguments_name, const SourceCodeExprInfo& src_info, COLTContext& ctx) noexcept;
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
    /// @brief The function's declaration
    PTR<FnDeclExpr> declaration;

  public:
    //No default copy constructor 
    FnDefExpr(const FnDefExpr&) = delete;
    //No default constructor
    FnDefExpr() = delete;
    /// @brief Destructor
    ~FnDefExpr() noexcept override = default;    
    /// @brief Creates function definition
    /// @param type The function type
    /// @param decl The declaration of the function
    /// @param body The body of the function
    /// @param src_info The source code information
    FnDefExpr(PTR<const Type> type, PTR<FnDeclExpr> decl, PTR<Expr> body, const SourceCodeExprInfo& src_info) noexcept
      : Expr(EXPR_FN_DEF, type, src_info), body(body), declaration(decl)
    {
      assert_true(type->is_fn(), "Expected a function type!");
    }

    /// @brief Sets the body of the function to 'body_p'
    /// @param body_p The new body of the function
    void set_body(PTR<Expr> body_p) noexcept { body = body_p; }

    /// @brief Get the expression to convert
    /// @return The expression to converse
    PTR<const Expr> get_body() const noexcept { return body; }

    /// @brief Returns the name of the global variable
    /// @return The name of the variable
    StringView get_name() const noexcept { return declaration->get_name(); }

    /// @brief Returns the parameter names
    /// @return View over the parameter names
    ContiguousView<StringView> get_params_name() const noexcept { return declaration->get_params_name(); }
    /// @brief Returns the parameter types
    /// @return View over the parameter types
    ContiguousView<PTR<const Type>> get_params_type() const noexcept { return declaration->get_params_type(); }
    /// @brief Returns the return type of the function
    /// @return The return type of the function
    PTR<const Type> get_return_type() const noexcept { return declaration->get_return_type(); }

    PTR<const FnDeclExpr> get_fn_decl() const noexcept { return declaration; }

    /// @brief Creates a FnDefExpr
    /// @param decl The declaration of the function
    /// @param body The body of the function
    /// @param src_info The source code information
    /// @param ctx The COLTContext to store the resulting expression
    /// @return Pointer to the created expression
    static PTR<Expr> CreateExpr(PTR<FnDeclExpr> decl, PTR<Expr> body, const SourceCodeExprInfo& src_info, COLTContext& ctx) noexcept;
    /// @brief Creates a FnDefExpr
    /// @param decl The declaration of the function
    /// @param src_info The source code information
    /// @param ctx The COLTContext to store the resulting expression
    /// @return Pointer to the created expression
    static PTR<Expr> CreateExpr(PTR<FnDeclExpr> decl, const SourceCodeExprInfo& src_info, COLTContext& ctx) noexcept;
  };

  class FnCallExpr
    final : public Expr
  {
  public:
    /// @brief Helper for dyn_cast and is_a
    static constexpr ExprID classof_v = EXPR_FN_CALL;

  private:
    /// @brief The arguments of the function
    SmallVector<PTR<Expr>, 4> arguments;
    /// @brief The function's declaration
    PTR<const FnDeclExpr> declaration;

  public:
    //No default copy constructor 
    FnCallExpr(const FnDefExpr&) = delete;
    //No default constructor
    FnCallExpr() = delete;
    /// @brief Destructor
    ~FnCallExpr() noexcept override = default;
    /// @brief Creates function definition
    /// @param decl The declaration of the function being called
    /// @param arguments The arguments to pass to the function
    /// @param src_info The source code information
    FnCallExpr(PTR<const FnDeclExpr> decl, SmallVector<PTR<Expr>, 4>&& arguments, const SourceCodeExprInfo& src_info) noexcept
      : Expr(EXPR_FN_CALL, decl->get_return_type(), src_info), arguments(std::move(arguments)), declaration(decl)
    {}

    /// @brief Returns declaration of the function
    /// @return The name of the variable
    PTR<const FnDeclExpr> get_fn_decl() const noexcept { return declaration; }

    /// @brief Returns the arguments of the function call
    /// @return View over the arguments
    ContiguousView<PTR<Expr>> get_arguments() const noexcept { return arguments.to_view(); }

    /// @brief Creates a function call
    // @param decl The declaration of the function being called
    /// @param arguments The arguments to pass to the function
    /// @param src_info The source code information
    /// @param ctx The COLTContext to store the resulting expression
    /// @return Pointer to the created expression
    static PTR<Expr> CreateExpr(PTR<const FnDeclExpr> decl, SmallVector<PTR<Expr>, 4>&& arguments, const SourceCodeExprInfo& src_info, COLTContext& ctx) noexcept;
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

  public:
    //No default copy constructor 
    ScopeExpr(const ScopeExpr&) = delete;
    //No default constructor
    ScopeExpr() = delete;
    /// @brief Destructor
    ~ScopeExpr() noexcept override = default;		
    /// @brief Constructs a ScopeExpr from an array of Expr*
    /// @param type The type of the resulting expression
    /// @param body_expr The Vector of expressions contained in the scope
    /// @param src_info The source code information
    ScopeExpr(PTR<const Type> type, Vector<PTR<Expr>>&& body_expr, const SourceCodeExprInfo& src_info) noexcept
      : Expr(EXPR_SCOPE, type, src_info), body_expr(std::move(body_expr)) {}

    /// @brief Sets the body of the scope to 'body'
    /// @param body The new body
    void set_body_array(Vector<PTR<Expr>>&& body) noexcept { body_expr = std::move(body); }

    /// @brief Get the expression to convert
    /// @return The expression to converse
    ContiguousView<PTR<Expr>> get_body_array() const noexcept { return body_expr.to_view(); }

    /// @brief Pushes an expression to the end of the scope
    /// @param expr The expression to push
    void push_back(PTR<Expr> expr) noexcept { body_expr.push_back(expr); }

    /// @brief Creates a ScopeExpr
    /// @param body The body of the ScopeExpr
    /// @param src_info The source code information
    /// @param ctx The COLTContext to store the resulting expression
    /// @return Pointer to the created expression
    static PTR<Expr> CreateExpr(Vector<PTR<Expr>>&& body, const SourceCodeExprInfo& src_info, COLTContext& ctx) noexcept;
  };

  /// @brief Represents a condition
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
  
  public:
    //No default copy constructor 
    ConditionExpr(const ConditionExpr&) = delete;
    //No default constructor
    ConditionExpr() = delete;
    /// @brief Destructor
    ~ConditionExpr() noexcept override = default;
    /// @brief Constructs a condition expression
    /// @param type The type of the resulting expression
    /// @param if_cond The if condition
    /// @param if_stmt The statement to evaluate if the if condition is true
    /// @param else_stmt The else statement, which can be null
    /// @param src_info The source code information
    ConditionExpr(PTR<const Type> type, PTR<Expr> if_cond, PTR<Expr> if_stmt, PTR<Expr> else_stmt, const SourceCodeExprInfo& src_info) noexcept
      : Expr(EXPR_CONDITION, type, src_info), if_cond(if_cond), if_stmt(if_stmt), else_stmt(else_stmt)
    {
      assert(if_cond->get_type()->is_builtin());
    }

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
    /// @param src_info The source code information
    /// @param ctx The COLTContext to store the resulting expression
    /// @return Pointer to the created expression
    static PTR<Expr> CreateExpr(PTR<Expr> if_cond, PTR<Expr> if_stmt, PTR<Expr> else_stmt, const SourceCodeExprInfo& src_info, COLTContext& ctx) noexcept;
  };

  /// @brief Represents a while loop
  class WhileLoopExpr
    final : public Expr
  {
  public:
    /// @brief Helper for dyn_cast and is_a
    static constexpr ExprID classof_v = EXPR_WHILE_LOOP;

  private:
    /// @brief The while condition
    PTR<Expr> condition;
    /// @brief The while body
    PTR<Expr> body;

  public:
    //No default copy constructor 
    WhileLoopExpr(const WhileLoopExpr&) = delete;
    //No default constructor
    WhileLoopExpr() = delete;
    /// @brief Destructor
    ~WhileLoopExpr() noexcept override = default;
    /// @brief Constructs a while loop expression
    /// @param type The type of the resulting expression
    /// @param condition The while condition
    /// @param body The body of the condition
    /// @param src_info The source code information
    WhileLoopExpr(PTR<const Type> type, PTR<Expr> condition, PTR<Expr> body, const SourceCodeExprInfo& src_info) noexcept
      : Expr(EXPR_WHILE_LOOP, type, src_info), condition(condition), body(body)
    {
      assert(condition->get_type()->is_builtin());
    }

    /// @brief Get the expression to convert
    /// @return The expression to converse
    PTR<const Expr> get_condition() const noexcept { return condition; }

    /// @brief Get the expression to convert
    /// @return The expression to converse
    PTR<const Expr> get_body() const noexcept { return body; }

    /// @brief Constructs a while loop expression
    /// @param condition The while condition
    /// @param body The body of the condition
    /// @param src_info The source code information
    /// @param ctx The COLTContext to store the resulting expression
    /// @return Pointer to the created expression
    static PTR<Expr> CreateExpr(PTR<Expr> condition, PTR<Expr> body, const SourceCodeExprInfo& src_info, COLTContext& ctx) noexcept;
  };

  /// @brief Represents a while loop
  class BreakContinueExpr
    final : public Expr
  {
  public:
    /// @brief Helper for dyn_cast and is_a
    static constexpr ExprID classof_v = EXPR_BREAK_CONTINUE;

  private:
    /// @brief True if 'break', false if 'continue'
    bool is_break_v;

  public:
    //No default copy constructor 
    BreakContinueExpr(const BreakContinueExpr&) = delete;
    //No default constructor
    BreakContinueExpr() = delete;
    /// @brief Destructor
    ~BreakContinueExpr() noexcept override = default;
    /// @brief Constructs a while loop expression
    /// @param type The type of the resulting expression
    /// @param src_info The source code information
    BreakContinueExpr(PTR<const Type> type, bool is_break_v, const SourceCodeExprInfo& src_info) noexcept
      : Expr(EXPR_BREAK_CONTINUE, type, src_info), is_break_v(is_break_v) {}

    bool is_break() const noexcept { return is_break_v; }
    bool is_continue() const noexcept { return !is_break_v; }

    /// @brief Constructs a while loop expression
    /// @param src_info The source code information
    /// @param ctx The COLTContext to store the resulting expression
    /// @return Pointer to the created expression
    static PTR<Expr> CreateExpr(bool is_break, const SourceCodeExprInfo& src_info, COLTContext& ctx) noexcept;
  };

  /// @brief Represents a while loop
  class NoOpExpr
    final : public Expr
  {
  public:
    /// @brief Helper for dyn_cast and is_a
    static constexpr ExprID classof_v = EXPR_NOP;

    //No default copy constructor 
    NoOpExpr(const NoOpExpr&) = delete;
    //No default constructor
    NoOpExpr() = delete;
    /// @brief Destructor
    ~NoOpExpr() noexcept override = default;
    /// @brief Constructs a while loop expression
    /// @param type The type of the resulting expression
    /// @param src_info The source code information
    NoOpExpr(PTR<const Type> type, const SourceCodeExprInfo& src_info) noexcept
      : Expr(EXPR_NOP, type, src_info) {}

    /// @brief Constructs a while loop expression
    /// @param src_info The source code information
    /// @param ctx The COLTContext to store the resulting expression
    /// @return Pointer to the created expression
    static PTR<Expr> CreateExpr(const SourceCodeExprInfo& src_info, COLTContext& ctx) noexcept;
  };
}

#endif //!HG_COLT_EXPR