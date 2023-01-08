/** @file colt_type.cpp
* Contains definition of functions declared in 'colt_type.h'.
*/

#include "colt_type.h"
#include "ast/colt_context.h"

namespace colt::lang
{
  PTR<Type> VoidType::CreateType(COLTContext& ctx) noexcept
  {
    return ctx.add_type(make_unique<VoidType>(true));
  }

  PTR<Type> VoidType::CreateType(bool is_const, COLTContext& ctx) noexcept
  {
    return ctx.add_type(make_unique<VoidType>(is_const));
  }
  
  bool BuiltInType::supports(BinaryOperator op) const noexcept
  {
    for (size_t i = 0; i < valid_op.get_size(); i++)
    {
      if (valid_op[i] == op)
        return true;
    }
    return false;
  }

  PTR<const Type> BuiltInType::clone_as_const(COLTContext& ctx) const noexcept
  {
    using fn_ptr = PTR<Type>(*)(bool, COLTContext&) noexcept;
    constexpr fn_ptr table[] = {
      &CreateBool, &CreateChar,
      &CreateU8, &CreateU16, &CreateU32, &CreateU64, &CreateU128,
      &CreateI8, &CreateI16, &CreateI32, &CreateI64, &CreateI128,
      &CreateF32, &CreateF64
    };
    assert_true(builtin_ID < lstring, "Invalid built-in ID!");
    return table[builtin_ID](true, ctx);
  }

  PTR<const Type> BuiltInType::clone_as_mut(COLTContext& ctx) const noexcept
  {
    using fn_ptr = PTR<Type>(*)(bool, COLTContext&) noexcept;
    constexpr fn_ptr table[] = {
      &CreateBool, &CreateChar,
      &CreateU8, &CreateU16, &CreateU32, &CreateU64, &CreateU128,
      &CreateI8, &CreateI16, &CreateI32, &CreateI64, &CreateI128,
      &CreateF32, &CreateF64
    };
    assert_true(builtin_ID < lstring, "Invalid built-in ID!");
    return table[builtin_ID](false, ctx);
  }

  PTR<Type> BuiltInType::CreateU8(bool is_const, COLTContext& ctx) noexcept
  {
    return ctx.add_type(make_unique<BuiltInType>(BuiltInID::U8, is_const,
      ContiguousView<BinaryOperator>{ BuiltInType::IntegralSupported, std::size(BuiltInType::IntegralSupported) },
      "mut u8" + (4 * as<u64>(is_const)))
    );
  }
  
  PTR<Type> BuiltInType::CreateU16(bool is_const, COLTContext& ctx) noexcept
  {
    return ctx.add_type(make_unique<BuiltInType>(BuiltInID::U16, is_const,
      ContiguousView<BinaryOperator>{ BuiltInType::IntegralSupported, std::size(BuiltInType::IntegralSupported) },
      "mut u16" + (4 * as<u64>(is_const)))
    );
  }
  
  PTR<Type> BuiltInType::CreateU32(bool is_const, COLTContext& ctx) noexcept
  {
    return ctx.add_type(make_unique<BuiltInType>(BuiltInID::U32, is_const,
      ContiguousView<BinaryOperator>{ BuiltInType::IntegralSupported, std::size(BuiltInType::IntegralSupported) },
      "mut u32" + (4 * as<u64>(is_const)))
    );
  }
  
  PTR<Type> BuiltInType::CreateU64(bool is_const, COLTContext& ctx) noexcept
  {
    return ctx.add_type(make_unique<BuiltInType>(BuiltInID::U64, is_const,
      ContiguousView<BinaryOperator>{ BuiltInType::IntegralSupported, std::size(BuiltInType::IntegralSupported) },
      "mut u64" + (4 * as<u64>(is_const)))
    );
  }
  
  PTR<Type> BuiltInType::CreateU128(bool is_const, COLTContext& ctx) noexcept
  {
    return ctx.add_type(make_unique<BuiltInType>(BuiltInID::U128, is_const,
      ContiguousView<BinaryOperator>{ BuiltInType::IntegralSupported, std::size(BuiltInType::IntegralSupported) },
      "mut u128" + (4 * as<u64>(is_const)))
    );
  }
  
  PTR<Type> BuiltInType::CreateI8(bool is_const, COLTContext& ctx) noexcept
  {
    return ctx.add_type(make_unique<BuiltInType>(BuiltInID::I8, is_const,
      ContiguousView<BinaryOperator>{ BuiltInType::IntegralSupported, std::size(BuiltInType::IntegralSupported) },
      "mut i8" + (4 * as<u64>(is_const)))
    );
  }
  
  PTR<Type> BuiltInType::CreateI16(bool is_const, COLTContext& ctx) noexcept
  {
    return ctx.add_type(make_unique<BuiltInType>(BuiltInID::I16, is_const,
      ContiguousView<BinaryOperator>{ BuiltInType::IntegralSupported, std::size(BuiltInType::IntegralSupported) },
      "mut i16" + (4 * as<u64>(is_const)))
    );
  }
  
  PTR<Type> BuiltInType::CreateI32(bool is_const, COLTContext& ctx) noexcept
  {
    return ctx.add_type(make_unique<BuiltInType>(BuiltInID::I32, is_const,
      ContiguousView<BinaryOperator>{ BuiltInType::IntegralSupported, std::size(BuiltInType::IntegralSupported) },
      "mut i32" + (4 * as<u64>(is_const)))
    );
  }
  
  PTR<Type> BuiltInType::CreateI64(bool is_const, COLTContext& ctx) noexcept
  {
    return ctx.add_type(make_unique<BuiltInType>(BuiltInID::I64, is_const,
      ContiguousView<BinaryOperator>{ BuiltInType::IntegralSupported, std::size(BuiltInType::IntegralSupported) },
      "mut i64" + (4 * as<u64>(is_const)))
    );
  }
  
  PTR<Type> BuiltInType::CreateI128(bool is_const, COLTContext& ctx) noexcept
  {
    return ctx.add_type(make_unique<BuiltInType>(BuiltInID::I128, is_const,
      ContiguousView<BinaryOperator>{ BuiltInType::IntegralSupported, std::size(BuiltInType::IntegralSupported) },
      "mut i128" + (4 * as<u64>(is_const)))
    );
  }
  
  PTR<Type> BuiltInType::CreateF32(bool is_const, COLTContext& ctx) noexcept
  {
    return ctx.add_type(make_unique<BuiltInType>(BuiltInID::F32, is_const,
      ContiguousView<BinaryOperator>{ BuiltInType::FloatingSupported, std::size(BuiltInType::FloatingSupported) },
      "mut float" + (4 * as<u64>(is_const)))
    );
  }
  
  PTR<Type> BuiltInType::CreateF64(bool is_const, COLTContext& ctx) noexcept
  {
    return ctx.add_type(make_unique<BuiltInType>(BuiltInID::F64, is_const,
      ContiguousView<BinaryOperator>{ BuiltInType::FloatingSupported, std::size(BuiltInType::FloatingSupported) },
      "mut double" + (4 * as<u64>(is_const)))
    );
  }
  
  PTR<Type> BuiltInType::CreateBool(bool is_const, COLTContext& ctx) noexcept
  {
    return ctx.add_type(make_unique<BuiltInType>(BuiltInID::BOOL, is_const,
      ContiguousView<BinaryOperator>{ BuiltInType::BoolSupported, std::size(BuiltInType::BoolSupported) },
      "mut bool" + (4 * as<u64>(is_const)))
    );
  }

  PTR<Type> BuiltInType::CreateChar(bool is_const, COLTContext& ctx) noexcept
  {
    return ctx.add_type(make_unique<BuiltInType>(BuiltInID::CHAR, is_const,
      ContiguousView<BinaryOperator>{ BuiltInType::CharSupported, std::size(BuiltInType::CharSupported) },
      "mut char" + (4 * as<u64>(is_const)))
    );
  }

  PTR<Type> BuiltInType::CreateLString(COLTContext& ctx) noexcept
  {
    return ctx.add_type(make_unique<BuiltInType>(BuiltInID::lstring, true,
      ContiguousView<BinaryOperator>{ BuiltInType::lstringSupported, std::size(BuiltInType::lstringSupported) },
      "lstring")
    );
  }
  
  PTR<Type> PtrType::CreatePtr(bool is_const, PTR<const Type> ptr_to, COLTContext& ctx) noexcept
  {
    auto str = String{ "mut PTR<" + (4 * as<u64>(is_const)) };
    str += ptr_to->get_name();
    str += ">";    
    return ctx.add_type(make_unique<PtrType>(is_const, ptr_to,
      ctx.add_str(std::move(str))
      ));
  }
  
  PTR<Type> FnType::CreateFn(PTR<const Type> return_type, SmallVector<PTR<const Type>, 4>&& args_type, bool is_vararg, COLTContext& ctx) noexcept
  {
    auto str = String{ "fn(" };
    if (args_type.is_not_empty())
      str += args_type.get_front()->get_name();
    for (size_t i = 1; i < args_type.get_size(); i++)
    {
      str += ", ";
      str += args_type.get_front()->get_name();
    }
    if (is_vararg)
      str += "var_arg";
    str += ")->";
    str += return_type->get_name();
    
    return ctx.add_type(make_unique<FnType>(return_type, std::move(args_type),
      is_vararg, ctx.add_str(std::move(str))));
  }

  PTR<Type> FnType::CreateFn(PTR<const Type> return_type, SmallVector<PTR<const Type>, 4>&& args_type, COLTContext& ctx) noexcept
  {
    auto str = String{ "fn(" };
    if (args_type.is_not_empty())
      str += args_type.get_front()->get_name();
    for (size_t i = 1; i < args_type.get_size(); i++)
    {
      str += ", ";
      str += args_type.get_front()->get_name();
    }
    str += ")->";
    str += return_type->get_name();
    
    return ctx.add_type(make_unique<FnType>(return_type, std::move(args_type),
      false, ctx.add_str(std::move(str))));
  }
  
  PTR<Type> ErrorType::CreateType(COLTContext& ctx) noexcept
  {
    return ctx.add_type(make_unique<ErrorType>());
  }  
  
  PTR<const Type> Type::clone_as_const(COLTContext& ctx) const noexcept
  {
    if (is_const())
      return this;
    
    switch (ID)
    {
    case Type::TYPE_ERROR:
      return this;
    case Type::TYPE_VOID:
      return VoidType::CreateType(true, ctx);
    case Type::TYPE_BUILTIN:
      return as<PTR<const BuiltInType>>(this)->clone_as_const(ctx);
    case Type::TYPE_PTR:
      return PtrType::CreatePtr(true,
        as<PTR<const PtrType>>(this)->get_type_to(), ctx);

    case Type::TYPE_ARRAY:
    case Type::TYPE_CLASS:
    default:
      colt_unreachable("Invalid conversion!");
    }
  }

  PTR<const Type> Type::clone_as_mut(COLTContext& ctx) const noexcept
  {
    if (!is_const())
      return this;

    switch (ID)
    {
    case Type::TYPE_ERROR:
      return this;
    case Type::TYPE_VOID:
      return VoidType::CreateType(false, ctx);
    case Type::TYPE_BUILTIN:
      return as<PTR<const BuiltInType>>(this)->clone_as_mut(ctx);
    case Type::TYPE_PTR:
      return PtrType::CreatePtr(false,
        as<PTR<const PtrType>>(this)->get_type_to(), ctx);

    case Type::TYPE_ARRAY:
    case Type::TYPE_CLASS:
    default:
      colt_unreachable("Invalid conversion!");
    }
  }

  bool Type::is_ptr_to_void() const noexcept
  {
    return ID == TYPE_PTR && as<PTR<const PtrType>>(this)->get_type_to()->is_void();
  }

  bool Type::is_semantically_integral() const noexcept
  {
    if (!is_builtin())
      return false;
    auto ret = as<PTR<const BuiltInType>>(this);
    return CHAR < ret->get_builtin_id()
      && ret->get_builtin_id() < F32;
  }

  bool Type::is_integral() const noexcept
  {
    if (!is_builtin())
      return false;
    auto ret = as<PTR<const BuiltInType>>(this);
    return ret->get_builtin_id() < F32;
  }

  bool Type::is_bool() const noexcept
  {
    if (!is_builtin())
      return false;
    auto ret = as<PTR<const BuiltInType>>(this);
    return ret->get_builtin_id() == BOOL;
  }

  bool Type::is_floating() const noexcept
  {
    if (!is_builtin())
      return false;
    auto ret = as<PTR<const BuiltInType>>(this);
    return ret->get_builtin_id() == F32
      || ret->get_builtin_id() == F64;
  }

  bool Type::is_f32() const noexcept
  {
    if (!is_builtin())
      return false;
    auto ret = as<PTR<const BuiltInType>>(this);
    return ret->get_builtin_id() == F32;
  }

  bool Type::is_f64() const noexcept
  {
    if (!is_builtin())
      return false;
    auto ret = as<PTR<const BuiltInType>>(this);
    return ret->get_builtin_id() == F64;
  }

  bool Type::is_signed_int() const noexcept
  {
    if (!is_builtin())
      return false;
    auto ret = as<PTR<const BuiltInType>>(this);
    return U64 < ret->get_builtin_id()
      && ret->get_builtin_id() < F32;
  }

  bool Type::is_signed() const noexcept
  {
    if (!is_builtin())
      return false;
    auto ret = as<PTR<const BuiltInType>>(this);
    return U64 < ret->get_builtin_id()
      && ret->get_builtin_id() < lstring;
  }

  bool Type::is_unsigned_int() const noexcept
  {
    if (!is_builtin())
      return false;
    auto ret = as<PTR<const BuiltInType>>(this);
    return ret->get_builtin_id() < I8;
  }

  bool Type::is_lstring() const noexcept
  {
    if (!is_builtin())
      return false;
    auto ret = as<PTR<const BuiltInType>>(this);
    return ret->get_builtin_id() == lstring;
  }

  bool Type::is_char() const noexcept
  {
    if (!is_builtin())
      return false;
    auto ret = as<PTR<const BuiltInType>>(this);
    return ret->get_builtin_id() == CHAR;
  }

  bool Type::is_equal(PTR<const Type> type) const noexcept
  {
    if (this->is_error() || type->is_error())
      return true;
    if (this->classof() != type->classof())
      return false;

    switch (classof())
    {
    case TYPE_VOID:
      return true;
    case TYPE_BUILTIN:
    {
      auto a = as<PTR<const BuiltInType>>(type);
      auto b = as<PTR<const BuiltInType>>(this);
      return a->get_builtin_id() == b->get_builtin_id();
    }
    case TYPE_PTR:
    {
      auto a = as<PTR<const PtrType>>(type);
      auto b = as<PTR<const PtrType>>(this);      
      return a->get_type_to()->is_equal(b->get_type_to());
    }
    case TYPE_FN:
    {
      auto a = as<PTR<const FnType>>(type);
      auto b = as<PTR<const FnType>>(this);
      if (!a->get_return_type()->is_equal(b->get_return_type())
        && a->get_params_type().get_size() != b->get_params_type().get_size())
        return false;
      for (size_t i = 0; i < a->get_params_type().get_size(); i++)
      {
        if (!a->get_params_type()[i]->is_equal(b->get_params_type()[i]))
          return false;
      }
      return true;
    }
    case TYPE_ARRAY:
    case TYPE_CLASS:
    default:
      colt_unreachable("Invalid type comparison!");
    }
  }
}
