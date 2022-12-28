#ifndef HG_COLT_MANGLE
#define HG_COLT_MANGLE

#include <util/colt_pch.h>
#include <type/colt_type.h>
#include <ast/colt_expr.h>

namespace colt::gen
{
  /// @brief Mangles a function name
  /// @param fn_name The function name
  /// @param ret The return type of the function
  /// @param args View over the arguments name
  /// @return String representing the mangled name
  String mangle(StringView fn_name, StringView ret, lang::TypeNameIter args) noexcept;

  /// @brief Mangles a function declaration
  /// @param fn_decl The function declaration to mangle
  /// @return String representing the mangled name
  String mangle(PTR<const lang::FnDeclExpr> fn_decl) noexcept;

  /// @brief Demangles a mangled function name
  /// @param mangled_name The mangled name to demangle
  /// @return Demangled name
  String demangle(StringView mangled_name) noexcept;
}

#endif //!HG_COLT_MANGLE