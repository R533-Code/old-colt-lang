/** @file colt_context.h
* Contains helpers to simplify storing types and expressions.
* The COLTContext class stores unique types and expressions, which allows
* dealing with pointers only. As the stored types and expressions are unique,
* comparing the pointers obtained through the context is enough to know if 2
* types/expressions are the same. This is an important optimization that is used
* throughout comparisons operator of types and expressions.
*/

#ifndef HG_COLT_CONTEXT
#define HG_COLT_CONTEXT

#include <util/colt_pch.h>

#include <ast/colt_expr.h>
#include <type/colt_type.h>

namespace colt::lang
{
  /// @brief Class responsible of holding Type and Expr used in the AST
  class COLTContext
  {
    /// @brief Saved String
    FlatList<String, 256> saved_str;
    /// @brief StableSet of types
    FlatList<UniquePtr<Type>> type_set;
    /// @brief StableSet of expressions
    FlatList<UniquePtr<Expr>, 256> expr_set;

  public:
    /// @brief Save an expression and returns a pointer to it
    /// @param expr The expression to add
    /// @return Pointer to the unique expression
    PTR<Expr> add_expr(UniquePtr<Expr>&& expr) noexcept
    {
      expr_set.push_back(std::move(expr));
      return expr_set.get_back().get_ptr();
    }

    /// @brief Save an type and returns a pointer to it
    /// @param type The type to add
    /// @return Pointer to the unique type
    PTR<Type> add_type(UniquePtr<Type>&& type) noexcept
    {
      type_set.push_back(std::move(type));
      return type_set.get_back().get_ptr();
    }

    /// @brief Saves a String and returns a StringView over it
    /// @param str The String to save
    /// @return StringView over the saved String
    StringView add_str(String&& str) noexcept
    {
      saved_str.push_back(std::move(str));
      return saved_str.get_back();
    }
  };
}

#endif //!HG_COLT_CONTEXT