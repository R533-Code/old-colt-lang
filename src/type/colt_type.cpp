/** @file colt_type.cpp
* Contains definition of functions declared in 'colt_type.h'.
*/

#include "colt_type.h"
#include "context/colt_context.h"

namespace colt::lang
{
  bool operator==(const Type& lhs, const Type& rhs) noexcept
  {
    if (lhs.classof() != rhs.classof() || lhs.is_mutable() != rhs.is_mutable())
      return false;

    switch (lhs.classof())
    {
    case Type::TYPE_VOID:
      return true;
    case Type::TYPE_BUILTIN:
      return static_cast<const BuiltInType&>(lhs) == static_cast<const BuiltInType&>(rhs);
    case Type::TYPE_PTR:
      return static_cast<const PtrType&>(lhs) == static_cast<const PtrType&>(rhs);
    case Type::TYPE_FN:
      return static_cast<const FnType&>(lhs) == static_cast<const FnType&>(rhs);
    default:
      colt_unreachable("Invalid classof for type!");
    }
  }

  bool operator==(const BuiltInType& lhs, const BuiltInType& rhs) noexcept
  {
    return lhs.builtin_ID == rhs.builtin_ID;
  }

  bool operator==(const PtrType& lhs, const PtrType& rhs) noexcept
  {
    return lhs.ptr_to == rhs.ptr_to;
  }

  bool operator==(const FnType& lhs, const FnType& rhs) noexcept
  {
    return lhs.return_type == rhs.return_type
      && lhs.args_type == rhs.args_type;
  }
  
  bool operator==(const UniquePtr<Type>& lhs, const UniquePtr<Type>& rhs) noexcept
  {
    if (lhs.get_ptr() == rhs.get_ptr())
      return true;
    if (!lhs || !rhs)
      return false;
    return *lhs == *rhs;
  }

  PTR<Type> VoidType::CreateType(COLTContext& ctx) noexcept
  {
    return ctx.add_type(make_unique<VoidType>());
  }
  
  constexpr bool BuiltInType::supports(BinaryOperator op) const noexcept
  {
    for (size_t i = 0; i < valid_op.get_size(); i++)
    {
      if (valid_op[i] == op)
        return true;
    }
    return false;
  }

  PTR<Type> BuiltInType::CreateU8(bool is_mut, COLTContext& ctx) noexcept
  {
    return ctx.add_type(make_unique<BuiltInType>(BuiltInID::U8, is_mut,
      ContiguousView<BinaryOperator>{ BuiltInType::IntegralSupported, std::size(BuiltInType::IntegralSupported) })
    );
  }
  
  PTR<Type> BuiltInType::CreateU16(bool is_mut, COLTContext& ctx) noexcept
  {
    return ctx.add_type(make_unique<BuiltInType>(BuiltInID::U16, is_mut,
      ContiguousView<BinaryOperator>{ BuiltInType::IntegralSupported, std::size(BuiltInType::IntegralSupported) })
    );
  }
  
  PTR<Type> BuiltInType::CreateU32(bool is_mut, COLTContext& ctx) noexcept
  {
    return ctx.add_type(make_unique<BuiltInType>(BuiltInID::U32, is_mut,
      ContiguousView<BinaryOperator>{ BuiltInType::IntegralSupported, std::size(BuiltInType::IntegralSupported) })
    );
  }
  
  PTR<Type> BuiltInType::CreateU64(bool is_mut, COLTContext& ctx) noexcept
  {
    return ctx.add_type(make_unique<BuiltInType>(BuiltInID::U64, is_mut,
      ContiguousView<BinaryOperator>{ BuiltInType::IntegralSupported, std::size(BuiltInType::IntegralSupported) })
    );
  }
  
  PTR<Type> BuiltInType::CreateU128(bool is_mut, COLTContext& ctx) noexcept
  {
    return ctx.add_type(make_unique<BuiltInType>(BuiltInID::U128, is_mut,
      ContiguousView<BinaryOperator>{ BuiltInType::IntegralSupported, std::size(BuiltInType::IntegralSupported) })
    );
  }
  
  PTR<Type> BuiltInType::CreateI8(bool is_mut, COLTContext& ctx) noexcept
  {
    return ctx.add_type(make_unique<BuiltInType>(BuiltInID::I8, is_mut,
      ContiguousView<BinaryOperator>{ BuiltInType::IntegralSupported, std::size(BuiltInType::IntegralSupported) })
    );
  }
  
  PTR<Type> BuiltInType::CreateI16(bool is_mut, COLTContext& ctx) noexcept
  {
    return ctx.add_type(make_unique<BuiltInType>(BuiltInID::I16, is_mut,
      ContiguousView<BinaryOperator>{ BuiltInType::IntegralSupported, std::size(BuiltInType::IntegralSupported) })
    );
  }
  
  PTR<Type> BuiltInType::CreateI32(bool is_mut, COLTContext& ctx) noexcept
  {
    return ctx.add_type(make_unique<BuiltInType>(BuiltInID::I32, is_mut,
      ContiguousView<BinaryOperator>{ BuiltInType::IntegralSupported, std::size(BuiltInType::IntegralSupported) })
    );
  }
  
  PTR<Type> BuiltInType::CreateI64(bool is_mut, COLTContext& ctx) noexcept
  {
    return ctx.add_type(make_unique<BuiltInType>(BuiltInID::I64, is_mut,
      ContiguousView<BinaryOperator>{ BuiltInType::IntegralSupported, std::size(BuiltInType::IntegralSupported) })
    );
  }
  
  PTR<Type> BuiltInType::CreateI128(bool is_mut, COLTContext& ctx) noexcept
  {
    return ctx.add_type(make_unique<BuiltInType>(BuiltInID::I128, is_mut,
      ContiguousView<BinaryOperator>{ BuiltInType::IntegralSupported, std::size(BuiltInType::IntegralSupported) })
    );
  }
  
  PTR<Type> BuiltInType::CreateF32(bool is_mut, COLTContext& ctx) noexcept
  {
    return ctx.add_type(make_unique<BuiltInType>(BuiltInID::F32, is_mut,
      ContiguousView<BinaryOperator>{ BuiltInType::FloatingSupported, std::size(BuiltInType::FloatingSupported) })
    );
  }
  
  PTR<Type> BuiltInType::CreateF64(bool is_mut, COLTContext& ctx) noexcept
  {
    return ctx.add_type(make_unique<BuiltInType>(BuiltInID::F64, is_mut,
      ContiguousView<BinaryOperator>{ BuiltInType::FloatingSupported, std::size(BuiltInType::FloatingSupported) })
    );
  }
  PTR<Type> BuiltInType::CreateBool(bool is_mut, COLTContext& ctx) noexcept
  {
    return ctx.add_type(make_unique<BuiltInType>(BuiltInID::BOOL, is_mut,
      ContiguousView<BinaryOperator>{ BuiltInType::BoolSupported, std::size(BuiltInType::BoolSupported) })
    );
  }
  
  PTR<Type> PtrType::CreatePtr(bool is_mut, PTR<Type> ptr_to, COLTContext& ctx) noexcept
  {
    return ctx.add_type(make_unique<PtrType>(is_mut, ptr_to));
  }
  
  PTR<Type> FnType::CreateFn(PTR<Type> return_type, SmallVector<PTR<Type>>&& args_type, COLTContext& ctx) noexcept
  {
    return ctx.add_type(make_unique<FnType>(return_type, std::move(args_type)));
  }
  
  PTR<Type> ErrorType::CreateType(COLTContext& ctx) noexcept
  {
    return ctx.add_type(make_unique<ErrorType>());
  }
}

namespace colt
{
  size_t hash(const lang::Type& type) noexcept
  {
    using namespace colt::lang;

    switch (type.classof())
    {
    case Type::TYPE_VOID:
      return 0;
    case Type::TYPE_BUILTIN:
      return GetHash(static_cast<const BuiltInType&>(type).get_builtin_id());
    case Type::TYPE_PTR:
      return GetHash(static_cast<const PtrType&>(type).get_type_to());
    case Type::TYPE_FN:
      return HashCombine(GetHash(static_cast<const FnType&>(type).get_return_type()),
        GetHash(static_cast<const FnType&>(type).get_params_type()));
    default:
      colt_unreachable("Invalid classof for type!");
    }
  }
}