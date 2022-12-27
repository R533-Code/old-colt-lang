/** @file colt_type.h
* Contains classes representing types in the front-end of the compiler.
* All classes present in this file support `dyn_cast` and `is_a`.
*/

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
      /// @brief ErrorType
      TYPE_ERROR,
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
    /// @brief Name of the type
    StringView name;
    /// @brief The ID of the expression
    TypeID ID;
    /// @brief True if the type is const
    bool is_const_v;

  public:
    Type() = delete;
    Type(const Type&) = delete;
    Type(Type&&) = delete;

    /// @brief Constructor
    /// @param ID The type ID
    /// @param is_const True if the type is const
    constexpr Type(TypeID ID, bool is_const, StringView name) noexcept
      : name(name), ID(ID), is_const_v(is_const) {}

    /// @brief Destructor
    virtual ~Type() noexcept = default;

    /// @brief Returns the actual type of the Expr
    /// @return The ExprID of the current expression
    constexpr TypeID classof() const noexcept { return ID; }

    /// @brief Check if the type is const.
    /// For VoidType or FnType, returns false.
    /// @return True if the type is const
    constexpr bool is_const() const noexcept { return is_const_v; }
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
    /// @brief Check if the type is error
    /// @return True if error
    constexpr bool is_error() const noexcept { return ID == TYPE_ERROR; } 
    /// @brief Returns the typename
    /// @return StringView over the typename
    constexpr StringView get_name() const noexcept { return name; }

    bool is_equal(PTR<const Type> type) const noexcept;
    bool is_equal_with_const(PTR<const Type> type) const noexcept;
  };

  /// @brief Represents an error.
  /// The use of an ErrorType class is to replace use of nullptr.
  class ErrorType
    final : public Type
  {
  public:
    /// @brief Helper for dyn_cast and is_a
    static constexpr TypeID classof_v = TYPE_ERROR;

    /// @brief No default constructor
    constexpr ErrorType() noexcept
      : Type(TYPE_ERROR, false, "<Error>") {}

    /// @brief Destructor
    ~ErrorType() noexcept override = default;

    /// @brief Creates a ErrorType
    /// @param ctx The COLTContext to store the resulting type
    /// @return Pointer to the created expression
    static PTR<Type> CreateType(COLTContext& ctx) noexcept;
  };

  /// @brief Void type
  class VoidType
    final : public Type
  {
  public:
    /// @brief Helper for dyn_cast and is_a
    static constexpr TypeID classof_v = TYPE_VOID;

    /// @brief No default constructor
    constexpr VoidType() noexcept
      : Type(TYPE_VOID, false, "void") {}
    
    /// @brief Destructor
    ~VoidType() noexcept override = default;

    /// @brief Creates a VoidType
    /// @param ctx The COLTContext to store the resulting type
    /// @return Pointer to the created expression
    static PTR<Type> CreateType(COLTContext& ctx) noexcept;
  };

  /// @brief Built in types
  class BuiltInType
    final : public Type
  {
  public:
    /// @brief Helper for dyn_cast and is_a
    static constexpr TypeID classof_v = TYPE_BUILTIN;

    /// @brief ID of the built-in type
    enum BuiltInID
      : u8
    {
      /// @brief Boolean, Unsigned 1-bit integer
      BOOL,
      /// @brief 8-bit ASCII char
      CHAR,
      /// @brief Unsigned 8-bit integer
      U8,
      /// @brief Unsigned 16-bit integer
      U16,
      /// @brief Unsigned 32-bit integer
      U32,
      /// @brief Unsigned 64-bit integer
      U64,
      /// @brief Unsigned 128-bit integer
      U128,
      /// @brief Signed 8-bit integer
      I8,
      /// @brief Signed 16-bit integer
      I16,
      /// @brief Signed 32-bit integer
      I32,
      /// @brief Signed 64-bit integer
      I64,
      /// @brief Signed 128-bit integer
      I128,
      /// @brief 32-bit floating point
      F32,
      /// @brief 64-bit floating point
      F64,      
    };
  
  private:
    /// @brief BinaryOperator supported by integral built-in types
    static constexpr BinaryOperator IntegralSupported[16] = {
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

    /// @brief BinaryOperator supported by floating point built-in types
    static constexpr BinaryOperator FloatingSupported[10] = {
      BinaryOperator::OP_SUM, BinaryOperator::OP_SUB,
      BinaryOperator::OP_MUL, BinaryOperator::OP_DIV,
      BinaryOperator::OP_EQUAL, BinaryOperator::OP_NOT_EQUAL,
      BinaryOperator::OP_GREAT, BinaryOperator::OP_GREAT_EQUAL,
      BinaryOperator::OP_LESS, BinaryOperator::OP_LESS_EQUAL      
    };

    /// @brief BinaryOperator supported by boolean
    static constexpr BinaryOperator BoolSupported[4] = {     
      BinaryOperator::OP_EQUAL, BinaryOperator::OP_NOT_EQUAL,      
      BinaryOperator::OP_BOOL_AND, BinaryOperator::OP_BOOL_OR,
    };

    /// @brief BinaryOperator supported by boolean
    static constexpr BinaryOperator CharSupported[2] = {
      BinaryOperator::OP_EQUAL, BinaryOperator::OP_NOT_EQUAL,
    };

    /// @brief BinaryOperator supported by boolean
    static constexpr BinaryOperator lstringSupported[2] = {
      BinaryOperator::OP_EQUAL, BinaryOperator::OP_NOT_EQUAL,
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

    /// @brief Creates a built-in type
    /// @param builtinID The type ID
    /// @param is_const True if const
    /// @param valid_op Array of possible binary operator
    constexpr BuiltInType(BuiltInID builtinID, bool is_const, ContiguousView<BinaryOperator> valid_op, StringView name) noexcept
      : Type(TYPE_BUILTIN, is_const, name), builtin_ID(builtinID), valid_op(valid_op) {}

    /// @brief Returns the built-in ID
    /// @return BuiltInID of the current type
    constexpr BuiltInID get_builtin_id() const noexcept { return builtin_ID; }
    /// @brief Check if the current type is any of the signed/unsigned built-in integers
    /// @return True if built-in integer
    constexpr bool is_integral() const noexcept { return builtin_ID < F32; }
    /// @brief Check if the current type is bool
    /// @return True if built-in boolean
    constexpr bool is_bool() const noexcept { return builtin_ID == BOOL; }
    /// @brief Check if the current type is an f32 or f64
    /// @return True if f32 or f64
    constexpr bool is_floating() const noexcept { return builtin_ID == F32 || builtin_ID == F64; }
    /// @brief Check if the current type is any of the signed built-in integers
    /// @return True if built-in signed integer
    constexpr bool is_signed_int() const noexcept { return U64 < builtin_ID && builtin_ID < F32; }
    /// @brief Check if the current type is any of the unsigned built-in integers
    /// @return True if built-in unsigned integer
    constexpr bool is_unsigned_int() const noexcept { return builtin_ID < I8; }

    /// @brief Check if the current type supports 'op' BinaryOperator
    /// @param op The operator to check for
    /// @return True if the current type supports 'op'
    bool supports(BinaryOperator op) const noexcept;

    /// @brief Creates a U8 type
    /// @param is_const True if const
    /// @param ctx The context to store the result
    /// @return Pointer to the resulting type
    static PTR<Type> CreateU8(bool is_const, COLTContext& ctx) noexcept;
    /// @brief Creates a U16 type
    /// @param is_const True if const
    /// @param ctx The context to store the result
    /// @return Pointer to the resulting type
    static PTR<Type> CreateU16(bool is_const, COLTContext& ctx) noexcept;
    /// @brief Creates a U32 type
    /// @param is_const True if const
    /// @param ctx The context to store the result
    /// @return Pointer to the resulting type
    static PTR<Type> CreateU32(bool is_const, COLTContext& ctx) noexcept;
    /// @brief Creates a U64 type
    /// @param is_const True if const
    /// @param ctx The context to store the result
    /// @return Pointer to the resulting type
    static PTR<Type> CreateU64(bool is_const, COLTContext& ctx) noexcept;
    /// @brief Creates a U128 type
    /// @param is_const True if const
    /// @param ctx The context to store the result
    /// @return Pointer to the resulting type
    static PTR<Type> CreateU128(bool is_const, COLTContext& ctx) noexcept;
    /// @brief Creates a I8 type
    /// @param is_const True if const
    /// @param ctx The context to store the result
    /// @return Pointer to the resulting type
    static PTR<Type> CreateI8(bool is_const, COLTContext& ctx) noexcept;
    /// @brief Creates a I16 type
    /// @param is_const True if const
    /// @param ctx The context to store the result
    /// @return Pointer to the resulting type
    static PTR<Type> CreateI16(bool is_const, COLTContext& ctx) noexcept;
    /// @brief Creates a I32 type
    /// @param is_const True if const
    /// @param ctx The context to store the result
    /// @return Pointer to the resulting type
    static PTR<Type> CreateI32(bool is_const, COLTContext& ctx) noexcept;
    /// @brief Creates a I64 type
    /// @param is_const True if const
    /// @param ctx The context to store the result
    /// @return Pointer to the resulting type
    static PTR<Type> CreateI64(bool is_const, COLTContext& ctx) noexcept;
    /// @brief Creates a I128 type
    /// @param is_const True if const
    /// @param ctx The context to store the result
    /// @return Pointer to the resulting type
    static PTR<Type> CreateI128(bool is_const, COLTContext& ctx) noexcept;
    /// @brief Creates a F32 type
    /// @param is_const True if const
    /// @param ctx The context to store the result
    /// @return Pointer to the resulting type
    static PTR<Type> CreateF32(bool is_const, COLTContext& ctx) noexcept;
    /// @brief Creates a F64 type
    /// @param is_const True if const
    /// @param ctx The context to store the result
    /// @return Pointer to the resulting type
    static PTR<Type> CreateF64(bool is_const, COLTContext& ctx) noexcept;
    /// @brief Creates a boolean type
    /// @param is_const True if const
    /// @param ctx The context to store the result
    /// @return Pointer to the resulting type
    static PTR<Type> CreateBool(bool is_const, COLTContext& ctx) noexcept;
    /// @brief Creates a char type
    /// @param is_const True if const
    /// @param ctx The context to store the result
    /// @return Pointer to the resulting type
    static PTR<Type> CreateChar(bool is_const, COLTContext& ctx) noexcept;
  };

  /// @brief Represents a pointer to a type
  class PtrType
    final : public Type
  {
  public:
    /// @brief Helper for dyn_cast and is_a
    static constexpr TypeID classof_v = TYPE_PTR;

  private:
    /// @brief The type pointed to
    PTR<const Type> ptr_to;

  public:
    /// @brief No default constructor
    PtrType() = delete;
    /// @brief Destructor
    ~PtrType() noexcept override = default;
    /// @brief Creates a pointer type
    /// @param is_const True if the pointer is const
    /// @param ptr_to The type pointed by the pointer
    constexpr PtrType(bool is_const, PTR<const Type> ptr_to, StringView name) noexcept
      : Type(TYPE_PTR, is_const, name), ptr_to(ptr_to) {}

    /// @brief Returns the type pointed to by the pointer
    /// @return The type pointed to by the pointer
    constexpr PTR<const Type> get_type_to() const noexcept { return ptr_to; }

    /// @brief Creates a pointer type
    /// @param is_const True if the pointer is const
    /// @param ptr_to The type pointed by the pointer
    /// @param ctx The COLTContext to store the resulting type
    /// @return Pointer to the resulting type
    static PTR<Type> CreatePtr(bool is_const, PTR<const Type> ptr_to, COLTContext& ctx) noexcept;
  };

  /// @brief Represents a function type
  class FnType
    final : public Type
  {
  public:
    /// @brief Helper for dyn_cast and is_a
    static constexpr TypeID classof_v = TYPE_FN;

  private:
    /// @brief The parameters' type
    SmallVector<PTR<const Type>, 4> args_type;
    /// @brief The function return type
    PTR<const Type> return_type;
    /// @brief True if the function accept c-style variadic arguments
    bool is_vararg;

  public:
    /// @brief No default constructor
    FnType() = delete;
    /// @brief Destructor
    ~FnType() noexcept override = default;
    /// @brief Creates a function type
    /// @param return_type Return type of the function
    /// @param args_type Parameters' type
    /// @param is_vararg True if accepts c-style variadic arguments
    constexpr FnType(PTR<const Type> return_type, SmallVector<PTR<const Type>, 4>&& args_type, bool is_vararg, StringView name) noexcept
      : Type(TYPE_FN, false, name), args_type(std::move(args_type)), return_type(return_type), is_vararg(is_vararg) {}

    /// @brief Returns the return type of the function
    /// @return Return type of the function
    constexpr PTR<const Type> get_return_type() const noexcept { return return_type; }

    /// @brief Returns the parameters' type of the function
    /// @return View over the parameters' type
    constexpr ContiguousView<PTR<const Type>> get_params_type() const noexcept { return args_type.to_view(); }
    /// @brief Check if the function supports c-style variadics
    /// @return True if it does
    constexpr bool is_varargs() const noexcept { return is_vararg; }

    /// @brief Creates a function type
    /// @param return_type Return type of the function
    /// @param args_type Parameters' type
    /// @param ctx The COLTContext to store the resulting type
    /// @return Pointer to the resulting type
    static PTR<Type> CreateFn(PTR<const Type> return_type, SmallVector<PTR<const Type>, 4>&& args_type, COLTContext& ctx) noexcept;
    /// @brief Creates a function type
    /// @param return_type Return type of the function
    /// @param args_type Parameters' type
    /// @param is_vararg True if accepts c-style variadic arguments
    /// @param ctx The COLTContext to store the resulting type
    /// @return Pointer to the resulting type
    static PTR<Type> CreateFn(PTR<const Type> return_type, SmallVector<PTR<const Type>, 4>&& args_type, bool is_vararg, COLTContext& ctx) noexcept;
  };

  template<typename T>
  constexpr bool is_cpp_equivalent(PTR<const Type> type) noexcept;

  template<typename T>
  constexpr PTR<const Type> from_cpp_equivalent(COLTContext& ctx) noexcept;

  namespace details
  {
    template<typename T, typename... Args>
    constexpr bool is_cpp_equivalent_arg(ContiguousView<PTR<const Type>> args) noexcept
    {
      if constexpr (sizeof...(Args) == 0)
        return is_cpp_equivalent<T>(args[sizeof...(Args)]);
      else
      {
        if (!is_cpp_equivalent<T>(args[sizeof...(Args)]))
          return false;
        args.pop_back();
        return is_cpp_equivalent_arg<Args...>(args);
      }
    }

    template<typename T>
    constexpr bool is_cpp_equivalent_fn(PTR<const FnType> ptr, T(*fn)(void)) noexcept
    {
      if (ptr->get_params_type().get_size() != 0)
        return false;
      return is_cpp_equivalent<T>(ptr->get_return_type());
    }

    template<typename T, typename T2, typename... Args>
    constexpr bool is_cpp_equivalent_fn(PTR<const FnType> ptr, T(*fn)(T2, Args...)) noexcept
    {
      if (ptr->get_params_type().get_size() != sizeof...(Args) + 1)
        return false;
      return is_cpp_equivalent<T>(ptr->get_return_type())
        && is_cpp_equivalent_arg<T2, Args...>(ptr->get_params_type());
    }

    template<typename T>
    constexpr bool is_cpp_equivalent_fn(PTR<const FnType> ptr, T(*fn)(...)) noexcept
    {
      if (ptr->get_params_type().get_size() != 0 || !ptr->is_varargs())
        return false;
      return is_cpp_equivalent<T>(ptr->get_return_type());
    }

    template<typename T, typename T2, typename... Args>
    constexpr bool is_cpp_equivalent_fn(PTR<const FnType> ptr, T(*fn)(T2, Args..., ...)) noexcept
    {
      if ((ptr->get_params_type().get_size() != sizeof...(Args) + 1) || !ptr->is_varargs())
        return false;
      return is_cpp_equivalent<T>(ptr->get_return_type())
        && is_cpp_equivalent_arg<T2, Args...>(ptr->get_params_type());
    }    

    template<typename T, typename... Args>
    constexpr PTR<const Type> from_cpp_equivalent_fn(T(*fn)(Args...), COLTContext& ctx) noexcept
    {
      SmallVector<PTR<const Type>, 4> args;
      (args.push_back(from_cpp_equivalent<Args>(ctx)), ...);
      return FnType::CreateFn(from_cpp_equivalent<T>(ctx), std::move(args), ctx);
    }

    template<typename T, typename... Args>
    constexpr PTR<const Type> from_cpp_equivalent_fn(T(*fn)(Args..., ...), COLTContext& ctx) noexcept
    {
      SmallVector<PTR<const Type>, 4> args;
      (args.push_back(from_cpp_equivalent<Args>(ctx)), ...);
      return FnType::CreateFn(from_cpp_equivalent<T>(ctx), std::move(args), true, ctx);
    }
  }

  template<typename T>
  /// @brief Check if a runtime colt type is equivalent to 'T'
  /// @tparam T The type to compare against
  /// @param type The type to compare
  /// @return True if 'T' and 'type' are equivalent
  constexpr bool is_cpp_equivalent(PTR<const Type> type) noexcept
  {
    if constexpr (std::is_function_v<T> && !std::is_pointer_v<T>)
      return is_cpp_equivalent<std::add_pointer_t<T>>(type);
    else if constexpr (std::is_pointer_v<T>
      && std::is_function_v<std::remove_pointer_t<T>>)
    {
      if (!is_a<FnType>(type))
        return false;
      T fn_ptr = nullptr;
      return details::is_cpp_equivalent_fn(as<PTR<const FnType>>(type), fn_ptr);
    }
    else if constexpr (std::is_pointer_v<T>)
    {
      if (!is_a<PtrType>(type))
        return false;
      return std::is_const_v<T> == type->is_const()
        && is_cpp_equivalent<std::remove_pointer_t<T>>(as<PTR<const PtrType>>(type)->get_type_to());
    }
    else if constexpr (std::is_void_v<T>)
    {
      if (!is_a<VoidType>(type))
        return false;
      return true;
    }
    else if constexpr (std::is_fundamental_v<T>)
    {
      if (!is_a<BuiltInType>(type) && std::is_const_v<T> != type->is_const())
        return false;
      if constexpr (std::is_same_v<std::decay_t<T>, i8>)
        return as<PTR<const BuiltInType>>(type)->get_builtin_id() == BuiltInType::BuiltInID::I8;
      else if constexpr (std::is_same_v<std::decay_t<T>, u8>)
        return as<PTR<const BuiltInType>>(type)->get_builtin_id() == BuiltInType::BuiltInID::U8;
      else if constexpr (std::is_same_v<std::decay_t<T>, i16>)
        return as<PTR<const BuiltInType>>(type)->get_builtin_id() == BuiltInType::BuiltInID::I16;
      else if constexpr (std::is_same_v<std::decay_t<T>, u16>)
        return as<PTR<const BuiltInType>>(type)->get_builtin_id() == BuiltInType::BuiltInID::U16;
      else if constexpr (std::is_same_v<std::decay_t<T>, i32>)
        return as<PTR<const BuiltInType>>(type)->get_builtin_id() == BuiltInType::BuiltInID::I32;
      else if constexpr (std::is_same_v<std::decay_t<T>, u32>)
        return as<PTR<const BuiltInType>>(type)->get_builtin_id() == BuiltInType::BuiltInID::U32;
      else if constexpr (std::is_same_v<std::decay_t<T>, i64>)
        return as<PTR<const BuiltInType>>(type)->get_builtin_id() == BuiltInType::BuiltInID::I64;
      else if constexpr (std::is_same_v<std::decay_t<T>, u64>)
        return as<PTR<const BuiltInType>>(type)->get_builtin_id() == BuiltInType::BuiltInID::U64;
      else if constexpr (std::is_same_v<std::decay_t<T>, f32>)
        return as<PTR<const BuiltInType>>(type)->get_builtin_id() == BuiltInType::BuiltInID::F32;
      else if constexpr (std::is_same_v<std::decay_t<T>, f64>)
        return as<PTR<const BuiltInType>>(type)->get_builtin_id() == BuiltInType::BuiltInID::F64;
      else if constexpr (std::is_same_v<std::decay_t<T>, bool>)
        return as<PTR<const BuiltInType>>(type)->get_builtin_id() == BuiltInType::BuiltInID::BOOL;
      /*else if constexpr (std::is_same_v<T, char>)
        return as<PTR<const BuiltInType>>(type)->get_builtin_id() == BuiltInType::BuiltInID::CHAR;*/
    }
    colt_unreachable("Unknown type!");
  }

  template<typename T>
  /// @brief Creates a runtime colt type matching 'T'
  /// @tparam T The type to convert to a colt type
  /// @param ctx The context in which to store the result
  /// @return Pointer to the equivalent type
  constexpr PTR<const Type> from_cpp_equivalent(COLTContext& ctx) noexcept
  {
    if constexpr (std::is_function_v<T> && !std::is_pointer_v<T>)
    {
      T* fn = nullptr;
      return details::from_cpp_equivalent_fn(fn, ctx);
    }
    else if constexpr (std::is_pointer_v<T>
      && std::is_function_v<std::remove_pointer_t<T>>)
    {
      T fn = nullptr;
      return details::from_cpp_equivalent_fn(fn, ctx);
    }
    else if constexpr (std::is_pointer_v<T>)
      return PtrType::CreatePtr(std::is_const_v<T>, from_cpp_equivalent<std::remove_pointer_t<T>>(ctx), ctx);
    else if constexpr (std::is_void_v<T>)
      return VoidType::CreateType(ctx);
    else if constexpr (std::is_fundamental_v<T>)
    {
      if constexpr (std::is_same_v<std::decay_t<T>, i8>)
        return BuiltInType::CreateI8(std::is_const_v<T>, ctx);
      else if constexpr (std::is_same_v<std::decay_t<T>, u8>)
        return BuiltInType::CreateU8(std::is_const_v<T>, ctx);
      else if constexpr (std::is_same_v<std::decay_t<T>, i16>)
        return BuiltInType::CreateI16(std::is_const_v<T>, ctx);
      else if constexpr (std::is_same_v<std::decay_t<T>, u16>)
        return BuiltInType::CreateU16(std::is_const_v<T>, ctx);
      else if constexpr (std::is_same_v<std::decay_t<T>, i32>)
        return BuiltInType::CreateI32(std::is_const_v<T>, ctx);
      else if constexpr (std::is_same_v<std::decay_t<T>, u32>)
        return BuiltInType::CreateU32(std::is_const_v<T>, ctx);
      else if constexpr (std::is_same_v<std::decay_t<T>, i64>)
        return BuiltInType::CreateI64(std::is_const_v<T>, ctx);
      else if constexpr (std::is_same_v<std::decay_t<T>, u64>)
        return BuiltInType::CreateU64(std::is_const_v<T>, ctx);
      else if constexpr (std::is_same_v<std::decay_t<T>, f32>)
        return BuiltInType::CreateF32(std::is_const_v<T>, ctx);
      else if constexpr (std::is_same_v<std::decay_t<T>, f64>)
        return BuiltInType::CreateF64(std::is_const_v<T>, ctx);
      else if constexpr (std::is_same_v<std::decay_t<T>, bool>)
        return BuiltInType::CreateBool(std::is_const_v<T>, ctx);
      /*else if constexpr (std::is_same_v<T, char>)
        return as<PTR<const BuiltInType>>(type)->get_builtin_id() == BuiltInType::BuiltInID::CHAR;*/
    }
    colt_unreachable("Unknown type!");
  }
}

#endif //!HG_COLT_TYPE