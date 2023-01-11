/** @file colt_builtin_id.h
* Contains enum that represent a Colt built-in type.
*/

#ifndef COLT_HG_BUILTIN_ID
#define COLT_HG_BUILTIN_ID

namespace colt::lang
{
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
    /// @brief Pointer to constant characters
    lstring,
    /// @brief byte (8-bit)
    byte,
    /// @brief word (16-bit)
    word,
    /// @brief double word (32-bit)
    dword,
    /// @brief quadruple word (64-bit)
    qword,
  };

  constexpr bool is_uint(BuiltInID id) noexcept
  {
    return id <= U128;
  }

  constexpr bool is_int(BuiltInID id) noexcept
  {
    return I8 <= id && id <= I128;
  }

  constexpr bool is_integral(BuiltInID id) noexcept
  {
    return id <= I128;
  }

  constexpr bool is_fpoint(BuiltInID id) noexcept
  {
    return id == F32 || id == F64;
  }

  constexpr bool is_bytes(BuiltInID id) noexcept
  {
    return byte <= id
      && id <= qword;
  }

}

#endif //!COLT_HG_BUILTIN_ID