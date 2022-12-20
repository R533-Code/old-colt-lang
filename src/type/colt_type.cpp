/** @file colt_type.cpp
* Contains definition of functions declared in 'colt_type.h'.
*/

#include "colt_type.h"
#include "context/colt_context.h"

namespace colt::lang
{
  PTR<Type> VoidType::CreateType(COLTContext& ctx) noexcept
  {
    return ctx.add_type(make_unique<VoidType>());
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

  PTR<Type> BuiltInType::CreateU8(bool is_const, COLTContext& ctx) noexcept
  {
    return ctx.add_type(make_unique<BuiltInType>(BuiltInID::U8, is_const,
      ContiguousView<BinaryOperator>{ BuiltInType::IntegralSupported, std::size(BuiltInType::IntegralSupported) })
    );
  }
  
  PTR<Type> BuiltInType::CreateU16(bool is_const, COLTContext& ctx) noexcept
  {
    return ctx.add_type(make_unique<BuiltInType>(BuiltInID::U16, is_const,
      ContiguousView<BinaryOperator>{ BuiltInType::IntegralSupported, std::size(BuiltInType::IntegralSupported) })
    );
  }
  
  PTR<Type> BuiltInType::CreateU32(bool is_const, COLTContext& ctx) noexcept
  {
    return ctx.add_type(make_unique<BuiltInType>(BuiltInID::U32, is_const,
      ContiguousView<BinaryOperator>{ BuiltInType::IntegralSupported, std::size(BuiltInType::IntegralSupported) })
    );
  }
  
  PTR<Type> BuiltInType::CreateU64(bool is_const, COLTContext& ctx) noexcept
  {
    return ctx.add_type(make_unique<BuiltInType>(BuiltInID::U64, is_const,
      ContiguousView<BinaryOperator>{ BuiltInType::IntegralSupported, std::size(BuiltInType::IntegralSupported) })
    );
  }
  
  PTR<Type> BuiltInType::CreateU128(bool is_const, COLTContext& ctx) noexcept
  {
    return ctx.add_type(make_unique<BuiltInType>(BuiltInID::U128, is_const,
      ContiguousView<BinaryOperator>{ BuiltInType::IntegralSupported, std::size(BuiltInType::IntegralSupported) })
    );
  }
  
  PTR<Type> BuiltInType::CreateI8(bool is_const, COLTContext& ctx) noexcept
  {
    return ctx.add_type(make_unique<BuiltInType>(BuiltInID::I8, is_const,
      ContiguousView<BinaryOperator>{ BuiltInType::IntegralSupported, std::size(BuiltInType::IntegralSupported) })
    );
  }
  
  PTR<Type> BuiltInType::CreateI16(bool is_const, COLTContext& ctx) noexcept
  {
    return ctx.add_type(make_unique<BuiltInType>(BuiltInID::I16, is_const,
      ContiguousView<BinaryOperator>{ BuiltInType::IntegralSupported, std::size(BuiltInType::IntegralSupported) })
    );
  }
  
  PTR<Type> BuiltInType::CreateI32(bool is_const, COLTContext& ctx) noexcept
  {
    return ctx.add_type(make_unique<BuiltInType>(BuiltInID::I32, is_const,
      ContiguousView<BinaryOperator>{ BuiltInType::IntegralSupported, std::size(BuiltInType::IntegralSupported) })
    );
  }
  
  PTR<Type> BuiltInType::CreateI64(bool is_const, COLTContext& ctx) noexcept
  {
    return ctx.add_type(make_unique<BuiltInType>(BuiltInID::I64, is_const,
      ContiguousView<BinaryOperator>{ BuiltInType::IntegralSupported, std::size(BuiltInType::IntegralSupported) })
    );
  }
  
  PTR<Type> BuiltInType::CreateI128(bool is_const, COLTContext& ctx) noexcept
  {
    return ctx.add_type(make_unique<BuiltInType>(BuiltInID::I128, is_const,
      ContiguousView<BinaryOperator>{ BuiltInType::IntegralSupported, std::size(BuiltInType::IntegralSupported) })
    );
  }
  
  PTR<Type> BuiltInType::CreateF32(bool is_const, COLTContext& ctx) noexcept
  {
    return ctx.add_type(make_unique<BuiltInType>(BuiltInID::F32, is_const,
      ContiguousView<BinaryOperator>{ BuiltInType::FloatingSupported, std::size(BuiltInType::FloatingSupported) })
    );
  }
  
  PTR<Type> BuiltInType::CreateF64(bool is_const, COLTContext& ctx) noexcept
  {
    return ctx.add_type(make_unique<BuiltInType>(BuiltInID::F64, is_const,
      ContiguousView<BinaryOperator>{ BuiltInType::FloatingSupported, std::size(BuiltInType::FloatingSupported) })
    );
  }
  
  PTR<Type> BuiltInType::CreateBool(bool is_const, COLTContext& ctx) noexcept
  {
    return ctx.add_type(make_unique<BuiltInType>(BuiltInID::BOOL, is_const,
      ContiguousView<BinaryOperator>{ BuiltInType::BoolSupported, std::size(BuiltInType::BoolSupported) })
    );
  }
  
  PTR<Type> PtrType::CreatePtr(bool is_const, PTR<const Type> ptr_to, COLTContext& ctx) noexcept
  {
    return ctx.add_type(make_unique<PtrType>(is_const, ptr_to));
  }
  
  PTR<Type> FnType::CreateFn(PTR<const Type> return_type, SmallVector<PTR<const Type>, 4>&& args_type, COLTContext& ctx) noexcept
  {
    return ctx.add_type(make_unique<FnType>(return_type, std::move(args_type)));
  }
  
  PTR<Type> ErrorType::CreateType(COLTContext& ctx) noexcept
  {
    return ctx.add_type(make_unique<ErrorType>());
  }  
}
