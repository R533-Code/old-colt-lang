/** @file qword_op.cpp
* Contains definition of functions declared in 'qword_op.h'.
*/

#include "qword_op.h"

namespace colt::op
{
  //BOOL, CHAR,
  //U8, U16, U32, U64, U128,
  //I8, I16, I32, I64, I128,
  //F32, F64, lstring,

  OpError uint_overflow_check_add(QWORD a, QWORD b, QWORD result, lang::BuiltInID id) noexcept
  {
    using namespace lang;

    switch (id)
    {
    case U8:
      //Check if bit after 8 lowest bit is set
      if (result.as<u64>() & (std::numeric_limits<u8>::max() + 1))
        return UNSIGNED_OVERFLOW;
      return NO_ERROR;
    case U16:
      if (result.as<u64>() & (std::numeric_limits<u16>::max() + 1))
        return UNSIGNED_OVERFLOW;
      return NO_ERROR;
    case U32:
      if (result.as<u64>() & (std::numeric_limits<u32>::max() + 1))
        return UNSIGNED_OVERFLOW;
      return NO_ERROR;
    case U64:
      if (details::max(a.as<u64>(), b.as<u64>()) > result.as<u64>())
        return UNSIGNED_OVERFLOW;
      return NO_ERROR;
    break; default:
      colt_unreachable("Invalid ID!");
    }
  }

  OpError uint_overflow_check_sub(QWORD a, QWORD b, lang::BuiltInID id) noexcept
  {
    using namespace lang;
    
    switch (id)
    {
    case U8:
    case U16:
    case U32:
    case U64:
      //Check if 'b' is greater than 'a'
      if (b.as<u64>() > a.as<u64>())
        return UNSIGNED_UNDERFLOW;
      return NO_ERROR;
    break; default:
      colt_unreachable("Invalid ID!");
    }
  }

  OpError uint_overflow_check_mul(QWORD a, QWORD b, QWORD result, lang::BuiltInID id) noexcept
  {
    using namespace lang;

    switch (id)
    {
    case U8:
      //Check if any bits over the 8 lowest bits are set
      if (result.as<u64>() & (std::numeric_limits<u64>::max() & ~static_cast<u64>(std::numeric_limits<u8>::max())))
        return UNSIGNED_OVERFLOW;
      return NO_ERROR;
    case U16:
      if (result.as<u64>() & (std::numeric_limits<u64>::max() & ~static_cast<u64>(std::numeric_limits<u16>::max())))
        return UNSIGNED_OVERFLOW;
      return NO_ERROR;
    case U32:
      if (result.as<u64>() & (std::numeric_limits<u64>::max() & ~static_cast<u64>(std::numeric_limits<u32>::max())))
        return UNSIGNED_OVERFLOW;
      return NO_ERROR;
    case U64:
      if (a.as<u64>() != 0 && result.as<u64>() / a.as<u64>() != b.as<u64>())
        return UNSIGNED_OVERFLOW;
      return NO_ERROR;
    break; default:
      colt_unreachable("Invalid ID!");
    }
  }

  ResultQWORD add(QWORD a, QWORD b, lang::BuiltInID id) noexcept
  {
    using namespace lang;

    QWORD result;
    switch (id)
    {
    case U8:
    case U16:
    case U32:
    case U64:
      result = a.as<u64>() + b.as<u64>();
      return { result.as<u64>(), uint_overflow_check_add(a, b, result, id) };
    break; case I8:
      result = a.as<i8>() + b.as<i8>();
    break; case I16:
      result = a.as<i16>() + b.as<i16>();
    break; case I32:
      result = a.as<i32>() + b.as<i32>();
    break; case I64:
      result = a.as<i64>() + b.as<i64>();
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
    case U16:
    case U32:
    case U64:
      result = a.as<u64>() + b.as<u64>();
      return { result.as<u64>(), uint_overflow_check_sub(a, b, id) };
    break; case I8:
      result = a.as<i8>() - b.as<i8>();
    break; case I16:
      result = a.as<i16>() - b.as<i16>();
    break; case I32:
      result = a.as<i32>() - b.as<i32>();
    break; case I64:
      result = a.as<i64>() - b.as<i64>();
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
    case U16:
    case U32:
    case U64:
      result = a.as<u64>() * b.as<u64>();
      return { result.as<u64>(), uint_overflow_check_mul(a, b, result, id) };
    break; case I8:
      result = a.as<i8>() * b.as<i8>();
    break; case I16:
      result = a.as<i16>() * b.as<i16>();
    break; case I32:
      result = a.as<i32>() * b.as<i32>();
    break; case I64:
      result = a.as<i64>() * b.as<i64>();
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
    case U16:
    case U32:
    case U64:
      result = a.as<u64>() / b.as<u64>();
    break; case I8:
      result = a.as<i8>() / b.as<i8>();
    break; case I16:
      result = a.as<i16>() / b.as<i16>();
    break; case I32:
      result = a.as<i32>() / b.as<i32>();
    break; case I64:
      result = a.as<i64>() / b.as<i64>();
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
      colt_unreachable("Invalid type for 'div'!");
    }
    return { result, NO_ERROR };
  }
  
  ResultQWORD bit_and(QWORD a, QWORD b, lang::BuiltInID id) noexcept
  {
    using namespace lang;

    assert_true(is_integral(id), "Expected an integer!");
    QWORD result = a.as<u64>() & b.as<u64>();
    return { result, NO_ERROR };
  }
  
  ResultQWORD bit_or(QWORD a, QWORD b, lang::BuiltInID id) noexcept
  {
    using namespace lang;

    assert_true(is_integral(id), "Expected an integer!");
    QWORD result = a.as<u64>() | b.as<u64>();
    return { result, NO_ERROR };
  }
  
  ResultQWORD bit_xor(QWORD a, QWORD b, lang::BuiltInID id) noexcept
  {
    using namespace lang;

    assert_true(is_integral(id), "Expected an integer!");
    QWORD result = a.as<u64>() ^ b.as<u64>();
    return { result, NO_ERROR };
  }

  ResultQWORD bit_not(QWORD a, lang::BuiltInID id) noexcept
  {
    using namespace lang;

    assert_true(is_integral(id), "Expected an integer!");
    QWORD result = ~a.as<u64>();
    return { result, NO_ERROR };
  }
  
  ResultQWORD shr(QWORD a, QWORD b, lang::BuiltInID id) noexcept
  {
    return ResultQWORD();
  }
  
  ResultQWORD shl(QWORD a, QWORD b, lang::BuiltInID id) noexcept
  {
    return ResultQWORD();
  }  
  
  ResultQWORD neg(QWORD a, lang::BuiltInID id) noexcept
  {
    return ResultQWORD();
  }

  ResultQWORD cnv(QWORD a, lang::BuiltInID from, lang::BuiltInID to) noexcept
  {
    return ResultQWORD();
  }
}