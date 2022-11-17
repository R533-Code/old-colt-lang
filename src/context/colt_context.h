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
    /// @brief StableSet of types
    StableSet<UniquePtr<Type>> type_set;
    /// @brief StableSet of expressions
    Vector<UniquePtr<Expr>> expr_set;

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
      return type_set.insert(std::move(type)).first->get_ptr();
    }
  };
}

#endif //!HG_COLT_CONTEXT