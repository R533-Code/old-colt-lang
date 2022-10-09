#ifndef HG_COLT_TYPE
#define HG_COLT_TYPE

#include <util/colt_pch.h>
#include <ast/colt_operators.h>

/// @brief Contains helpers for types
namespace colt::lang
{
  //Forward declaration
  class COLTContext;

  /// @brief Abstract base class of all expressions
  class Type
  {
  public:
    /// @brief The ID of an Expr, which allows to cast to the real type
    enum TypeID
      : u8
    {
      /// @brief Type
      TYPE_BASE,
      /// @brief VoidType
      TYPE_VOID,
      /// @brief BuiltInType
      TYPE_BUILTIN,
      /// @brief PtrType
      TYPE_PTR,
      /// @brief FnType
      TYPE_FN,
      /// @brief ArrayType
      TYPE_ARRAY,
      /// @brief ClassType
      TYPE_CLASS
    };

    /// @brief Helper for dyn_cast and is_a
    static constexpr TypeID classof_v = TYPE_BASE;

  private:

    /// @brief The ID of the expression
    TypeID ID;
    /// @brief True if the type is mutable
    bool is_mut;

  public:
    Type() = delete;
    Type(const Type&) = delete;
    Type(Type&&) = delete;

    /// @brief Constructor
    /// @param type The type of the expression	
    constexpr Type(TypeID ID, bool is_mut) noexcept
      : ID(ID), is_mut(is_mut) {}

    /// @brief Destructor
    virtual ~Type() noexcept = default;

    /// @brief Returns the actual type of the Expr
    /// @return The ExprID of the current expression
    constexpr TypeID classof() const noexcept { return ID; }

    /// @brief Check if the type is mutable.
    /// For VoidType or FnType, returns false.
    /// @return True if the type is mutable
    constexpr bool is_mutable() const noexcept { return is_mut; }
    /// @brief Check if the type is void
    /// @return True if is void
    constexpr bool is_void() const noexcept { return ID == TYPE_VOID; }
    /// @brief Check if the type is a pointer
    /// @return True if is pointer
    constexpr bool is_ptr() const noexcept { return ID == TYPE_PTR; }
    /// @brief Check if the type is a function
    /// @return True if function
    constexpr bool is_fn() const noexcept { return ID == TYPE_FN; }
    /// @brief Check if the type is an array
    /// @return True if array
    constexpr bool is_array() const noexcept { return ID == TYPE_ARRAY; }
    /// @brief Check if the type is built-in
    /// @return True if built-in
    constexpr bool is_builtin() const noexcept { return ID == TYPE_BUILTIN; }

    /// @brief Compares two Type.
    /// This function dispatches to the right operator== depending on the true
    /// type of lhs and rhs.
    /// @param lhs The left hand side
    /// @param rhs The right hand side
    /// @return True if both are equal
    friend bool operator==(const Type& lhs, const Type& rhs) noexcept;
  };

  /// @brief Compares 2 UniquePtr of Type.
  /// @param lhs The left hand side
  /// @param rhs The right hand side
  /// @return True if equal
  bool operator==(const UniquePtr<Type>& lhs, const UniquePtr<Type>& rhs) noexcept;

  class VoidType
    final : public Type
  {
  public:
    /// @brief Helper for dyn_cast and is_a
    static constexpr TypeID classof_v = TYPE_VOID;

    /// @brief No default constructor
    constexpr VoidType() noexcept
      : Type(TYPE_VOID, false) {}
    
    /// @brief Destructor
    ~VoidType() noexcept override = default;

    /// @brief Creates a VoidType
    /// @param ctx The COLTContext to store the resulting type
    /// @return Pointer to the created expression
    static PTR<Type> CreateType(COLTContext& ctx) noexcept;
  };

  class BuiltInType
    final : public Type
  {
  public:
    /// @brief Helper for dyn_cast and is_a
    static constexpr TypeID classof_v = TYPE_BUILTIN;

    enum BuiltInID
      : u8
    {
      U8, U16, U32, U64, U128,
      I8, I16, I32, I64, I128,
      F32, F64,
      BOOL
    };
  
  private:
    static constexpr BinaryOperator IntegralSupported[] = {
      BinaryOperator::OP_SUM, BinaryOperator::OP_SUB,
      BinaryOperator::OP_MUL, BinaryOperator::OP_DIV,
      BinaryOperator::OP_MOD,
      BinaryOperator::OP_EQUAL, BinaryOperator::OP_NOT_EQUAL,
      BinaryOperator::OP_GREAT, BinaryOperator::OP_GREAT_EQUAL,
      BinaryOperator::OP_LESS, BinaryOperator::OP_LESS_EQUAL,
      BinaryOperator::OP_BIT_AND, BinaryOperator::OP_BIT_OR,
      BinaryOperator::OP_BIT_XOR,
      BinaryOperator::OP_BIT_LSHIFT, BinaryOperator::OP_BIT_RSHIFT
    };

    static constexpr BinaryOperator FloatingSupported[] = {
      BinaryOperator::OP_SUM, BinaryOperator::OP_SUB,
      BinaryOperator::OP_MUL, BinaryOperator::OP_DIV,
      BinaryOperator::OP_EQUAL, BinaryOperator::OP_NOT_EQUAL,
      BinaryOperator::OP_GREAT, BinaryOperator::OP_GREAT_EQUAL,
      BinaryOperator::OP_LESS, BinaryOperator::OP_LESS_EQUAL      
    };

    static constexpr BinaryOperator BoolSupported[] = {     
      BinaryOperator::OP_EQUAL, BinaryOperator::OP_NOT_EQUAL,      
      BinaryOperator::OP_BOOL_AND, BinaryOperator::OP_BOOL_OR,
    };

    /// @brief The ID of the built-in type
    BuiltInID builtin_ID;
    /// @brief View of array of possible binary operators
    ContiguousView<BinaryOperator> valid_op;
  
  public:
    /// @brief No default constructor
    BuiltInType() = delete;
    /// @brief Destructor
    ~BuiltInType() noexcept override = default;

    constexpr BuiltInType(BuiltInID builtinID, bool is_mut, ContiguousView<BinaryOperator> valid_op) noexcept
      : Type(TYPE_BUILTIN, is_mut), builtin_ID(builtinID), valid_op(valid_op) {}

    constexpr BuiltInID get_builtin_id() const noexcept { return builtin_ID; }
    constexpr bool is_integral() const noexcept { return builtin_ID < F32; }
    constexpr bool is_bool() const noexcept { return builtin_ID == BOOL; }
    constexpr bool is_floating() const noexcept { return builtin_ID == F32 || builtin_ID == F64; }
    constexpr bool is_signed_int() const noexcept { return builtin_ID - 5 < I8; }
    constexpr bool is_unsigned_int() const noexcept { return builtin_ID < I8; }

    static PTR<Type> CreateU8(bool is_mut, COLTContext& ctx) noexcept;
    static PTR<Type> CreateU16(bool is_mut, COLTContext& ctx) noexcept;
    static PTR<Type> CreateU32(bool is_mut, COLTContext& ctx) noexcept;
    static PTR<Type> CreateU64(bool is_mut, COLTContext& ctx) noexcept;
    static PTR<Type> CreateU128(bool is_mut, COLTContext& ctx) noexcept;
    static PTR<Type> CreateI8(bool is_mut, COLTContext& ctx) noexcept;
    static PTR<Type> CreateI16(bool is_mut, COLTContext& ctx) noexcept;
    static PTR<Type> CreateI32(bool is_mut, COLTContext& ctx) noexcept;
    static PTR<Type> CreateI64(bool is_mut, COLTContext& ctx) noexcept;
    static PTR<Type> CreateI128(bool is_mut, COLTContext& ctx) noexcept;
    static PTR<Type> CreateF32(bool is_mut, COLTContext& ctx) noexcept;
    static PTR<Type> CreateF64(bool is_mut, COLTContext& ctx) noexcept;
    static PTR<Type> CreateBool(bool is_mut, COLTContext& ctx) noexcept;

    /// @brief Compares 2 BuiltInType, without checking their mutability
    /// @param lhs The left hand side
    /// @param rhs The right hand side
    /// @return True if their BuiltInID are equal
    friend bool operator==(const BuiltInType& lhs, const BuiltInType& rhs) noexcept;
  };

  class PtrType
    final : public Type
  {
  public:
    /// @brief Helper for dyn_cast and is_a
    static constexpr TypeID classof_v = TYPE_PTR;

  private:
    /// @brief The type pointed to
    PTR<Type> ptr_to;

  public:
    /// @brief No default constructor
    PtrType() = delete;
    /// @brief Destructor
    ~PtrType() noexcept override = default;
    /// @brief Creates a pointer type
    /// @param is_mut True if the pointer is mutable
    /// @param ptr_to The type pointed by the pointer
    constexpr PtrType(bool is_mut, PTR<Type> ptr_to) noexcept
      : Type(TYPE_PTR, is_mut), ptr_to(ptr_to) {}

    /// @brief Returns the type pointed to by the pointer
    /// @return The type pointed to by the pointer
    constexpr PTR<const Type> get_type_to() const noexcept { return ptr_to; }

    /// @brief Creates a pointer type
    /// @param is_mut True if the pointer is mutable
    /// @param ptr_to The type pointed by the pointer
    /// @param ctx The COLTContext to store the resulting type
    /// @return Pointer to the resulting type
    static PTR<Type> CreatePtr(bool is_mut, PTR<Type> ptr_to, COLTContext& ctx) noexcept;

    /// @brief Compares 2 pointers type, without checking for mutability
    /// @param lhs The left hand side
    /// @param rhs The right hand side
    /// @return True if the type pointed to is the same
    friend bool operator==(const PtrType& lhs, const PtrType& rhs) noexcept;
  };

  class FnType
    final : public Type
  {
  public:
    /// @brief Helper for dyn_cast and is_a
    static constexpr TypeID classof_v = TYPE_FN;

  private:
    /// @brief The parameters' type
    SmallVector<PTR<Type>> args_type;
    /// @brief The function return type
    PTR<Type> return_type;

  public:
    /// @brief No default constructor
    FnType() = delete;
    /// @brief Destructor
    ~FnType() noexcept override = default;
    /// @brief Creates a function type
    /// @param return_type Return type of the function
    /// @param args_type Parameters' type
    constexpr FnType(PTR<Type> return_type, SmallVector<PTR<Type>>&& args_type) noexcept
      : Type(TYPE_FN, false), args_type(std::move(args_type)), return_type(return_type) {}

    /// @brief Returns the return type of the function
    /// @return Return type of the function
    constexpr PTR<const Type> get_return_type() const noexcept { return return_type; }

    /// @brief Returns the parameters' type of the function
    /// @return View over the parameters' type
    constexpr ContiguousView<PTR<Type>> get_params_type() const noexcept { return args_type.to_view(); }

    /// @brief Creates a function type
    /// @param return_type Return type of the function
    /// @param args_type Parameters' type
    /// @param ctx The COLTContext to store the resulting type
    /// @return Pointer to the resulting type
    static PTR<Type> CreateFn(PTR<Type> return_type, SmallVector<PTR<Type>>&& args_type, COLTContext& ctx) noexcept;

    /// @brief Compares 2 pointers type, without checking for mutability
    /// @param lhs The left hand side
    /// @param rhs The right hand side
    /// @return True if the type pointed to is the same
    friend bool operator==(const FnType& lhs, const FnType& rhs) noexcept;
  };
}

namespace colt
{
  size_t hash(const lang::Type& expr) noexcept;
}

#endif //!HG_COLT_TYPE