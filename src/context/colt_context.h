#ifndef HG_COLT_CONTEXT
#define HG_COLT_CONTEXT

#include <util/colt_pch.h>

#include <ast/colt_expr.h>
#include <type/colt_type.h>

namespace colt::lang
{
  class COLTContext
  {
    //StableSet<UniquePtr<Type>> type_set;
    StableSet<UniquePtr<Expr>> expr_set;

  public:
    
    PTR<Expr> add_expr(UniquePtr<Expr>&& expr) noexcept
    {
      return expr_set.insert(std::move(expr)).first->get_ptr();
    }
  };
}

#endif //!HG_COLT_CONTEXT