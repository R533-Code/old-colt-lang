/** @file qword_op.cpp
* Contains definition of functions declared in 'qword_op.h'.
*/

#include "qword_op.h"

namespace colt::op
{
  template<typename T>
  constexpr OpError IntOpToOpError(colt::IntOpResult res) noexcept
  {
    if constexpr (std::is_signed_v<T>)
    {
      switch (res)
      {
      case colt::OP_VALID:
        return OpError::NO_ERROR;
      case colt::OP_OVERFLOW:
        return OpError::SIGNED_OVERFLOW;
      case colt::OP_UNDERFLOW:
        return OpError::SIGNED_UNDERFLOW;
      default:
        colt_unreachable("Invalid enum!");
      }
    }
    else
    {
      switch (res)
      {
      case colt::OP_VALID:
        return OpError::NO_ERROR;
      case colt::OP_OVERFLOW:
        return OpError::UNSIGNED_OVERFLOW;
      case colt::OP_UNDERFLOW:
        return OpError::UNSIGNED_UNDERFLOW;
      default:
        colt_unreachable("Invalid enum!");
      }
    }
  }
  
  //BOOL, CHAR,
  //U8, U16, U32, U64, U128,
  //I8, I16, I32, I64, I128,
  //F32, F64, lstring,

  OpError shift_sizeof_check(QWORD sh_by, lang::BuiltInID id) noexcept
  {
    using namespace lang;

    switch (id)
    {
    case colt::lang::U8:
    case colt::lang::I8:
      if (sh_by.as<u64>() >= sizeof(u8))
        return SHIFT_BY_GRE_SIZEOF;
      return NO_ERROR;
    case colt::lang::U16:
    case colt::lang::I16:
      if (sh_by.as<u64>() >= sizeof(u16))
        return SHIFT_BY_GRE_SIZEOF;
      return NO_ERROR;
    case colt::lang::U32:
    case colt::lang::I32:
      if (sh_by.as<u64>() >= sizeof(u32))
        return SHIFT_BY_GRE_SIZEOF;
      return NO_ERROR;
    case colt::lang::U64:
    case colt::lang::I64:
      if (sh_by.as<u64>() >= sizeof(u64))
        return SHIFT_BY_GRE_SIZEOF;
      return NO_ERROR;
    break; default:
      colt_unreachable("Expected integral!");
    }
  }

  const char* OpErrorToStrExplain(OpError err) noexcept
  {
    switch (err)
    {
    case NO_ERROR:
      return "No errors detected!";
    case DIV_BY_ZERO:
      return "Integral division by zero!";
    case SHIFT_BY_GRE_SIZEOF:
      return "Shift by value greater than bits size!";
    case UNSIGNED_OVERFLOW:
      return "Unsigned overflow detected!";
    case UNSIGNED_UNDERFLOW:
      return "Unsigned underflow detected!";
    case SIGNED_OVERFLOW:
      return "Signed overflow detected!";
    case SIGNED_UNDERFLOW:
      return "Signed underflow detected!";
    case WAS_NAN:
      return "Floating point operation evaluates to NaN!";
    case RET_NAN:
      return "Floating point operation evaluates to NaN!";
    default:
      colt_unreachable("Invalid error!");
    }
  }

  ResultQWORD add(QWORD a, QWORD b, lang::BuiltInID id) noexcept
  {
    using namespace lang;

    QWORD result;
    switch (id)
    {
    case U8:
    {
      u8 ret;
      return { ret, IntOpToOpError<u8>(colt::add(a.as<u8>(), b.as<u8>(), ret)) };
    }
    case U16:
    {
      u16 ret;
      return { ret, IntOpToOpError<u8>(colt::add(a.as<u16>(), b.as<u16>(), ret)) };
    }
    case U32:
    {
      u32 ret;
      return { ret, IntOpToOpError<u8>(colt::add(a.as<u32>(), b.as<u32>(), ret)) };
    }
    case U64:
    {
      u64 ret;
      return { ret, IntOpToOpError<u8>(colt::add(a.as<u64>(), b.as<u64>(), ret)) };
    }
    case I8:
    {
      i8 ret;
      return { ret, IntOpToOpError<i8>(colt::add(a.as<i8>(), b.as<i8>(), ret)) };
    }
    case I16:
    {
      i16 ret;
      return { ret, IntOpToOpError<i8>(colt::add(a.as<i16>(), b.as<i16>(), ret)) };
    }
    case I32:
    {
      i32 ret;
      return { ret, IntOpToOpError<i8>(colt::add(a.as<i32>(), b.as<i32>(), ret)) };
    }
    case I64:
    {
      i64 ret;
      return { ret, IntOpToOpError<i8>(colt::add(a.as<i64>(), b.as<i64>(), ret)) };
    }
    break; case F32:
      if (std::isnan(a.as<f32>()))
        return { a, WAS_NAN };
      if (std::isnan(b.as<f32>()))
        return { b, WAS_NAN };
      result = a.as<f32>() + b.as<f32>();
      if (std::isnan(result.as<f32>()))
        return { result, RET_NAN };
    break; case F64:
      if (std::isnan(a.as<f64>()))
        return { a, WAS_NAN };
      if (std::isnan(b.as<f64>()))
        return { b, WAS_NAN };
      result = a.as<f64>() + b.as<f64>();
      if (std::isnan(result.as<f64>()))
        return { result, RET_NAN };
    break; default:
      colt_unreachable("Invalid type for 'add'!");
    }
    return { result, NO_ERROR };
  }
  
  ResultQWORD sub(QWORD a, QWORD b, lang::BuiltInID id) noexcept
  {
    using namespace lang;

    QWORD result;
    switch (id)
    {
    case U8:
    {
      u8 ret;
      return { ret, IntOpToOpError<u8>(colt::sub(a.as<u8>(), b.as<u8>(), ret)) };
    }
    case U16:
    {
      u16 ret;
      return { ret, IntOpToOpError<u8>(colt::sub(a.as<u16>(), b.as<u16>(), ret)) };
    }
    case U32:
    {
      u32 ret;
      return { ret, IntOpToOpError<u8>(colt::sub(a.as<u32>(), b.as<u32>(), ret)) };
    }
    case U64:
    {
      u64 ret;
      return { ret, IntOpToOpError<u8>(colt::sub(a.as<u64>(), b.as<u64>(), ret)) };
    }
    case I8:
    {
      i8 ret;
      return { ret, IntOpToOpError<i8>(colt::sub(a.as<i8>(), b.as<i8>(), ret)) };
    }
    case I16:
    {
      i16 ret;
      return { ret, IntOpToOpError<i8>(colt::sub(a.as<i16>(), b.as<i16>(), ret)) };
    }
    case I32:
    {
      i32 ret;
      return { ret, IntOpToOpError<i8>(colt::sub(a.as<i32>(), b.as<i32>(), ret)) };
    }
    case I64:
    {
      i64 ret;
      return { ret, IntOpToOpError<i8>(colt::sub(a.as<i64>(), b.as<i64>(), ret)) };
    }
    break; case F32:
      if (std::isnan(a.as<f32>()))
        return { a, WAS_NAN };
      if (std::isnan(b.as<f32>()))
        return { b, WAS_NAN };
      result = a.as<f32>() - b.as<f32>();
      if (std::isnan(result.as<f32>()))
        return { result, RET_NAN };
    break; case F64:
      if (std::isnan(a.as<f64>()))
        return { a, WAS_NAN };
      if (std::isnan(b.as<f64>()))
        return { b, WAS_NAN };
      result = a.as<f64>() - b.as<f64>();
      if (std::isnan(result.as<f64>()))
        return { result, RET_NAN };
    break; default:
      colt_unreachable("Invalid type for 'sub'!");
    }
    return { result, NO_ERROR };
  }
  
  ResultQWORD mul(QWORD a, QWORD b, lang::BuiltInID id) noexcept
  {
    using namespace lang;

    QWORD result;
    switch (id)
    {
    case U8:
    {
      u8 ret;
      return { ret, IntOpToOpError<u8>(colt::mul(a.as<u8>(), b.as<u8>(), ret)) };
    }
    case U16:
    {
      u16 ret;
      return { ret, IntOpToOpError<u8>(colt::mul(a.as<u16>(), b.as<u16>(), ret)) };
    }
    case U32:
    {
      u32 ret;
      return { ret, IntOpToOpError<u8>(colt::mul(a.as<u32>(), b.as<u32>(), ret)) };
    }
    case U64:
    {
      u64 ret;
      return { ret, IntOpToOpError<u8>(colt::mul(a.as<u64>(), b.as<u64>(), ret)) };
    }
    case I8:
    {
      i8 ret;
      return { ret, IntOpToOpError<i8>(colt::mul(a.as<i8>(), b.as<i8>(), ret)) };
    }
    case I16:
    {
      i16 ret;
      return { ret, IntOpToOpError<i8>(colt::mul(a.as<i16>(), b.as<i16>(), ret)) };
    }
    case I32:
    {
      i32 ret;
      return { ret, IntOpToOpError<i8>(colt::mul(a.as<i32>(), b.as<i32>(), ret)) };
    }
    case I64:
    {
      i64 ret;
      return { ret, IntOpToOpError<i8>(colt::mul(a.as<i64>(), b.as<i64>(), ret)) };
    }
    break; case F32:
      if (std::isnan(a.as<f32>()))
        return { a, WAS_NAN };
      if (std::isnan(b.as<f32>()))
        return { b, WAS_NAN };
      result = a.as<f32>() * b.as<f32>();
      if (std::isnan(result.as<f32>()))
        return { result, RET_NAN };
    break; case F64:
      if (std::isnan(a.as<f64>()))
        return { a, WAS_NAN };
      if (std::isnan(b.as<f64>()))
        return { b, WAS_NAN };
      result = a.as<f64>() * b.as<f64>();
      if (std::isnan(result.as<f64>()))
        return { result, RET_NAN };
    break; default:
      colt_unreachable("Invalid type for 'mul'!");
    }
    return { result, NO_ERROR };
  }
  
  ResultQWORD div(QWORD a, QWORD b, lang::BuiltInID id) noexcept
  {
    using namespace lang;

    if (is_integral(id) && b.as<u64>() == 0)
      return { QWORD{}, DIV_BY_ZERO };

    QWORD result;
    switch (id)
    {
    case U8:
    {
      u8 ret;
      return { ret, IntOpToOpError<u8>(colt::div(a.as<u8>(), b.as<u8>(), ret)) };
    }
    case U16:
    {
      u16 ret;
      return { ret, IntOpToOpError<u8>(colt::div(a.as<u16>(), b.as<u16>(), ret)) };
    }
    case U32:
    {
      u32 ret;
      return { ret, IntOpToOpError<u8>(colt::div(a.as<u32>(), b.as<u32>(), ret)) };
    }
    case U64:
    {
      u64 ret;
      return { ret, IntOpToOpError<u8>(colt::div(a.as<u64>(), b.as<u64>(), ret)) };
    }
    case I8:
    {
      i8 ret;
      return { ret, IntOpToOpError<i8>(colt::div(a.as<i8>(), b.as<i8>(), ret)) };
    }
    case I16:
    {
      i16 ret;
      return { ret, IntOpToOpError<i8>(colt::div(a.as<i16>(), b.as<i16>(), ret)) };
    }
    case I32:
    {
      i32 ret;
      return { ret, IntOpToOpError<i8>(colt::div(a.as<i32>(), b.as<i32>(), ret)) };
    }
    case I64:
    {
      i64 ret;
      return { ret, IntOpToOpError<i8>(colt::div(a.as<i64>(), b.as<i64>(), ret)) };
    }
    break; case F32:
      if (std::isnan(a.as<f32>()))
        return { a, WAS_NAN };
      if (std::isnan(b.as<f32>()))
        return { b, WAS_NAN };
      result = a.as<f32>() / b.as<f32>();
      if (std::isnan(result.as<f32>()))
        return { result, RET_NAN };
    break; case F64:
      if (std::isnan(a.as<f64>()))
        return { a, WAS_NAN };
      if (std::isnan(b.as<f64>()))
        return { b, WAS_NAN };
      result = a.as<f64>() / b.as<f64>();
      if (std::isnan(result.as<f64>()))
        return { result, RET_NAN };
    break; default:
      colt_unreachable("Invalid type for 'div'!");
    }
    return { result, NO_ERROR };
  }
  
  ResultQWORD mod(QWORD a, QWORD b, lang::BuiltInID id) noexcept
  {
    using namespace lang;

    if (is_integral(id) && b.as<u64>() == 0)
      return { QWORD{}, DIV_BY_ZERO };

    QWORD result;
    switch (id)
    {
    case U8:
    case U16:
    case U32:
    case U64:
      result = a.as<u64>() % b.as<u64>();
    break; case I8:
      result = a.as<i8>() % b.as<i8>();
    break; case I16:
      result = a.as<i16>() % b.as<i16>();
    break; case I32:
      result = a.as<i32>() % b.as<i32>();
    break; case I64:
      result = a.as<i64>() % b.as<i64>();
    break; default:
      colt_unreachable("Invalid type for 'mod'!");
    }
    return { result, NO_ERROR };
  }
  
  ResultQWORD bit_and(QWORD a, QWORD b, lang::BuiltInID id) noexcept
  {
    using namespace lang;

    assert_true(is_integral(id) || is_bytes(id), "Expected an integer!");
    QWORD result = a.as<u64>() & b.as<u64>();
    return { result, NO_ERROR };
  }
  
  ResultQWORD bit_or(QWORD a, QWORD b, lang::BuiltInID id) noexcept
  {
    using namespace lang;

    assert_true(is_integral(id) || is_bytes(id), "Expected an integer!");
    QWORD result = a.as<u64>() | b.as<u64>();
    return { result, NO_ERROR };
  }
  
  ResultQWORD bit_xor(QWORD a, QWORD b, lang::BuiltInID id) noexcept
  {
    using namespace lang;

    assert_true(is_integral(id) || is_bytes(id), "Expected an integer!");
    QWORD result = a.as<u64>() ^ b.as<u64>();
    return { result, NO_ERROR };
  }

  ResultQWORD bit_not(QWORD a, lang::BuiltInID id) noexcept
  {
    using namespace lang;

    assert_true(is_integral(id) || is_bytes(id), "Expected an integer!");
    QWORD result = ~a.as<u64>();
    return { result, NO_ERROR };
  }
  
  ResultQWORD shr(QWORD a, QWORD b, lang::BuiltInID id) noexcept
  {
    using namespace lang;

    assert_true(is_integral(id) || is_bytes(id), "Expected an integer!");
    QWORD result = a.as<u64>() >> b.as<u64>();    
    
    return { result, shift_sizeof_check(b, id) };
  }
  
  ResultQWORD shl(QWORD a, QWORD b, lang::BuiltInID id) noexcept
  {
    using namespace lang;

    assert_true(is_integral(id) || is_bytes(id), "Expected an integer!");
    QWORD result = a.as<u64>() << b.as<u64>();
    
    return { result, shift_sizeof_check(b, id) };
  }

  ResultQWORD bool_and(QWORD a, QWORD b, lang::BuiltInID id) noexcept
  {
    assert_true(id == lang::BOOL, "Expected a bool type");
    QWORD result = a.as<bool>() && b.as<bool>();
    
    return { result, NO_ERROR };
  }

  ResultQWORD bool_or(QWORD a, QWORD b, lang::BuiltInID id) noexcept
  {
    assert_true(id == lang::BOOL, "Expected a bool type");
    QWORD result = a.as<bool>() || b.as<bool>();

    return { result, NO_ERROR };
  }

  ResultQWORD eq(QWORD a, QWORD b, lang::BuiltInID id) noexcept
  {
    using namespace lang;

    QWORD result;
    switch (id)
    {
    case colt::lang::BOOL:
    case colt::lang::CHAR:
    case colt::lang::U8:
    case colt::lang::U16:
    case colt::lang::U32:
    case colt::lang::U64:
    case colt::lang::I8:
    case colt::lang::I16:
    case colt::lang::I32:
    case colt::lang::I64:
    case colt::lang::byte:
    case colt::lang::word:
    case colt::lang::dword:
    case colt::lang::qword:
      result = a.as<u64>() == b.as<u64>();
    break; case colt::lang::F32:
      if (std::isnan(a.as<f32>()))
        return { a, WAS_NAN };
      if (std::isnan(b.as<f32>()))
        return { b, WAS_NAN };
      result = a.as<f32>() == b.as<f32>();
      if (std::isnan(result.as<f32>()))
        return { result, RET_NAN };
    break; case colt::lang::F64:
      if (std::isnan(a.as<f64>()))
        return { a, WAS_NAN };
      if (std::isnan(b.as<f64>()))
        return { b, WAS_NAN };
      result = a.as<f64>() == b.as<f64>();
      if (std::isnan(result.as<f64>()))
        return { result, RET_NAN };
    break; default:
      colt_unreachable("Invalid ID!");
    }
    return { result, NO_ERROR };
  }

  ResultQWORD neq(QWORD a, QWORD b, lang::BuiltInID id) noexcept
  {
    auto res = eq(a, b, id);
    res.first = !res.first.as<bool>();
    return res;
  }

  ResultQWORD le(QWORD a, QWORD b, lang::BuiltInID id) noexcept
  {
    using namespace lang;

    QWORD result;
    switch (id)
    {
    case colt::lang::CHAR:
    case colt::lang::U8:
    case colt::lang::U16:
    case colt::lang::U32:
    case colt::lang::U64:
      result = a.as<u64>() < b.as<u64>();
    break; case colt::lang::I8:
      result = a.as<i8>() < b.as<i8>();
    break; case colt::lang::I16:
      result = a.as<i16>() < b.as<i16>();
    break; case colt::lang::I32:
      result = a.as<i32>() < b.as<i32>();
    break; case colt::lang::I64:
      result = a.as<i64>() < b.as<i64>();
    break; case colt::lang::F32:
      if (std::isnan(a.as<f32>()))
        return { a, WAS_NAN };
      if (std::isnan(b.as<f32>()))
        return { b, WAS_NAN };
      result = a.as<f32>() < b.as<f32>();
      if (std::isnan(result.as<f32>()))
        return { result, RET_NAN };
    break; case colt::lang::F64:
      if (std::isnan(a.as<f64>()))
        return { a, WAS_NAN };
      if (std::isnan(b.as<f64>()))
        return { b, WAS_NAN };
      result = a.as<f64>() < b.as<f64>();
      if (std::isnan(result.as<f64>()))
        return { result, RET_NAN };
    break; default:
      colt_unreachable("Invalid ID!");
    }
    return { result, NO_ERROR };
  }

  ResultQWORD leq(QWORD a, QWORD b, lang::BuiltInID id) noexcept
  {
    using namespace lang;

    QWORD result;
    switch (id)
    {
    case colt::lang::CHAR:
    case colt::lang::U8:
    case colt::lang::U16:
    case colt::lang::U32:
    case colt::lang::U64:
      result = a.as<u64>() <= b.as<u64>();
    break; case colt::lang::I8:
      result = a.as<i8>() <= b.as<i8>();
    break; case colt::lang::I16:
      result = a.as<i16>() <= b.as<i16>();
    break; case colt::lang::I32:
      result = a.as<i32>() <= b.as<i32>();
    break; case colt::lang::I64:
      result = a.as<i64>() <= b.as<i64>();
    break; case colt::lang::F32:
      if (std::isnan(a.as<f32>()))
        return { a, WAS_NAN };
      if (std::isnan(b.as<f32>()))
        return { b, WAS_NAN };
      result = a.as<f32>() <= b.as<f32>();
      if (std::isnan(result.as<f32>()))
        return { result, RET_NAN };
    break; case colt::lang::F64:
      if (std::isnan(a.as<f64>()))
        return { a, WAS_NAN };
      if (std::isnan(b.as<f64>()))
        return { b, WAS_NAN };
      result = a.as<f64>() <= b.as<f64>();
      if (std::isnan(result.as<f64>()))
        return { result, RET_NAN };
    break; default:
      colt_unreachable("Invalid ID!");
    }
    return { result, NO_ERROR };
  }

  ResultQWORD ge(QWORD a, QWORD b, lang::BuiltInID id) noexcept
  {
    using namespace lang;

    QWORD result;
    switch (id)
    {
    case colt::lang::CHAR:
    case colt::lang::U8:
    case colt::lang::U16:
    case colt::lang::U32:
    case colt::lang::U64:
      result = a.as<u64>() > b.as<u64>();
    break; case colt::lang::I8:
      result = a.as<i8>() > b.as<i8>();
    break; case colt::lang::I16:
      result = a.as<i16>() > b.as<i16>();
    break; case colt::lang::I32:
      result = a.as<i32>() > b.as<i32>();
    break; case colt::lang::I64:
      result = a.as<i64>() > b.as<i64>();
    break; case colt::lang::F32:
      if (std::isnan(a.as<f32>()))
        return { a, WAS_NAN };
      if (std::isnan(b.as<f32>()))
        return { b, WAS_NAN };
      result = a.as<f32>() > b.as<f32>();
      if (std::isnan(result.as<f32>()))
        return { result, RET_NAN };
    break; case colt::lang::F64:
      if (std::isnan(a.as<f64>()))
        return { a, WAS_NAN };
      if (std::isnan(b.as<f64>()))
        return { b, WAS_NAN };
      result = a.as<f64>() > b.as<f64>();
      if (std::isnan(result.as<f64>()))
        return { result, RET_NAN };
    break; default:
      colt_unreachable("Invalid ID!");
    }
    return { result, NO_ERROR };
  }

  ResultQWORD geq(QWORD a, QWORD b, lang::BuiltInID id) noexcept
  {
    using namespace lang;

    QWORD result;
    switch (id)
    {
    case colt::lang::CHAR:
    case colt::lang::U8:
    case colt::lang::U16:
    case colt::lang::U32:
    case colt::lang::U64:
      result = a.as<u64>() >= b.as<u64>();
    break; case colt::lang::I8:
      result = a.as<i8>() >= b.as<i8>();
    break; case colt::lang::I16:
      result = a.as<i16>() >= b.as<i16>();
    break; case colt::lang::I32:
      result = a.as<i32>() >= b.as<i32>();
    break; case colt::lang::I64:
      result = a.as<i64>() >= b.as<i64>();
    break; case colt::lang::F32:
      if (std::isnan(a.as<f32>()))
        return { a, WAS_NAN };
      if (std::isnan(b.as<f32>()))
        return { b, WAS_NAN };
      result = a.as<f32>() >= b.as<f32>();
      if (std::isnan(result.as<f32>()))
        return { result, RET_NAN };
    break; case colt::lang::F64:
      if (std::isnan(a.as<f64>()))
        return { a, WAS_NAN };
      if (std::isnan(b.as<f64>()))
        return { b, WAS_NAN };
      result = a.as<f64>() >= b.as<f64>();
      if (std::isnan(result.as<f64>()))
        return { result, RET_NAN };
    break; default:
      colt_unreachable("Invalid ID!");
    }
    return { result, NO_ERROR };
  }
  
  ResultQWORD neg(QWORD a, lang::BuiltInID id) noexcept
  {
    using namespace lang;

    QWORD result;
    switch (id)
    {
    break; case I8:
      result = -a.as<i8>();
    break; case I16:
      result = -a.as<i16>();
    break; case I32:
      result = -a.as<i32>();
    break; case I64:
      result = -a.as<i64>();
    break; case F32:
      if (std::isnan(a.as<f32>()))
        return { a, WAS_NAN };
      result = -a.as<f32>();
      if (std::isnan(result.as<f32>()))
        return { result, RET_NAN };
    break; case F64:
      if (std::isnan(a.as<f64>()))
        return { a, WAS_NAN };
      result = -a.as<f64>();
      if (std::isnan(result.as<f64>()))
        return { result, RET_NAN };
    break; default:
      colt_unreachable("Invalid type for 'neg'!");
    }
    return { result, NO_ERROR };
  }

  ResultQWORD cnv(QWORD a, lang::BuiltInID from, lang::BuiltInID to) noexcept
  {
    return ResultQWORD();
  }
  
  QWORD_bin_ins_t getInstFromBinaryOperator(lang::BinaryOperator op) noexcept
  {
    constexpr QWORD_bin_ins_t op_array[] = {
      &add, &sub, &mul, &div, &mod,
      &bit_and, &bit_or, &bit_xor,
      &shl, &shr, &bool_and, &bool_or,
      &le, &leq, &ge, &geq, &neq, &eq
    };
    assert_true(op < lang::BinaryOperator::OP_ASSIGN, "Invalid operator!");
    return op_array[static_cast<u64>(op)];    
  }
}