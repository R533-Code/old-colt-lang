/** @file qword_op.h
* Contains function for QWORD operations.
*/

#ifndef COLT_HG_QWORD_OP
#define COLT_HG_QWORD_OP

#include <utility>
#include <cmath>
#include <limits>
#include <ast/colt_operators.h>
#include <type/colt_builtin_id.h>
#include <colt/utility/Typedefs.h>

namespace colt::op
{
  namespace details
  {
    template<typename T>
    constexpr T max(T a, T b) noexcept
    {
      return a > b ? a : b;
    }

    template<typename T>
    constexpr T min(T a, T b) noexcept
    {
      return a > b ? b : a;
    }
  }

  enum OpError
  {
    /// @brief No error
    NO_ERROR,
    /// @brief Division or modulo by 0
    DIV_BY_ZERO,
    /// @brief Shift by a size greater than bits size
    SHIFT_BY_GRE_SIZEOF,
    /// @brief Unsigned overflow
    UNSIGNED_OVERFLOW,
    /// @brief Unsigned underflow
    UNSIGNED_UNDERFLOW,
    /// @brief Signed overflow
    SIGNED_OVERFLOW,
    /// @brief Signed underflow
    SIGNED_UNDERFLOW,
    /// @brief The floating point was NaN
    WAS_NAN,
    /// @brief The returned value is NaN
    RET_NAN,
  };

  /// @brief Converts an 'OpError' to a string explaining it
  /// @param err The OpError to explain
  /// @return String explaining the error
  const char* OpErrorToStrExplain(OpError err) noexcept;

  /// @brief The result of any operation
  using ResultQWORD = std::pair<QWORD, OpError>;

  /***************** BINARY *****************/

  ResultQWORD add(QWORD a, QWORD b, lang::BuiltInID id) noexcept;
  ResultQWORD sub(QWORD a, QWORD b, lang::BuiltInID id) noexcept;
  ResultQWORD mul(QWORD a, QWORD b, lang::BuiltInID id) noexcept;
  ResultQWORD div(QWORD a, QWORD b, lang::BuiltInID id) noexcept;
  ResultQWORD mod(QWORD a, QWORD b, lang::BuiltInID id) noexcept;
  ResultQWORD bit_and(QWORD a, QWORD b, lang::BuiltInID id) noexcept;
  ResultQWORD bit_or(QWORD a, QWORD b, lang::BuiltInID id) noexcept;
  ResultQWORD bit_xor(QWORD a, QWORD b, lang::BuiltInID id) noexcept;
  ResultQWORD bit_not(QWORD a, lang::BuiltInID id) noexcept;
  ResultQWORD shr(QWORD a, QWORD b, lang::BuiltInID id) noexcept;
  ResultQWORD shl(QWORD a, QWORD b, lang::BuiltInID id) noexcept;
  ResultQWORD bool_and(QWORD a, QWORD b, lang::BuiltInID id) noexcept;
  ResultQWORD bool_or(QWORD a, QWORD b, lang::BuiltInID id) noexcept;
  ResultQWORD  eq(QWORD a, QWORD b, lang::BuiltInID id) noexcept;
  ResultQWORD neq(QWORD a, QWORD b, lang::BuiltInID id) noexcept;
  ResultQWORD  ge(QWORD a, QWORD b, lang::BuiltInID id) noexcept;
  ResultQWORD geq(QWORD a, QWORD b, lang::BuiltInID id) noexcept;
  ResultQWORD  le(QWORD a, QWORD b, lang::BuiltInID id) noexcept;
  ResultQWORD leq(QWORD a, QWORD b, lang::BuiltInID id) noexcept;
  
  /***************** UNARY *****************/

  ResultQWORD neg(QWORD a, lang::BuiltInID id) noexcept;
  
  /***************** CONVERSIONS *****************/
  
  ResultQWORD cnv(QWORD a, lang::BuiltInID from, lang::BuiltInID to) noexcept;

  /// @brief QWORD binary instruction function type
  using QWORD_bin_ins_t = ResultQWORD(*)(QWORD, QWORD, lang::BuiltInID) noexcept;

  /// @brief Returns the instruction function representing a BinaryOperator
  /// @param op The binary operator
  /// @return Function pointer representing the BinaryOperator
  QWORD_bin_ins_t getInstFromBinaryOperator(lang::BinaryOperator op) noexcept;
}

#endif //!COLT_HG_QWORD_OP