/** @file dyn_cast.h
* Contains helpers for inheritances.
* As sometimes down-casting a base pointer to a derived pointer is useful,
* but dynamic_cast requires RTTI, this header provides utilities to simplify
* and make down-casting faster, through `dyn_cast`.
* To support `dyn_cast`, the base class and each derived class should implement:
* - static constexpr classof_v, which represents the unique ID of the class,
* - classof() method, which returns a unique ID for the real class,
* Example:
* ```c++
* class Base
* {
* public:
*   enum type_of
*   {
*     BASE, DERIVED_A
*   };
* 
*   static constexpr type_of classof_v = BASE;
*   virtual type_of classof() noexcept { return BASE; }
* };
* 
* class DerivedA
*   : public Base
* {
* public:
*   static constexpr type_of classof_v = DERIVED_A;
*   type_of classof() noexcept override { return DERIVED_A; }
* };
* ```
*/

#ifndef HG_COLT_DYN_CAST
#define HG_COLT_DYN_CAST

#include <type_traits>
#include "colt_config.h"

namespace colt
{
  namespace traits
  {
    template<typename T, typename = std::void_t<>>
    /// @brief Check if a type can be casted using 'colt::dyn_cast'
    /// @tparam T The type to check for
    /// @tparam  SFINAE helper
    struct is_dyn_castable
    {
      /// @brief True if dyn_cast can be used on T
      static constexpr bool value = false;
    };

    template<typename T>
    /// @brief Check if a type can be casted using 'colt::dyn_cast'
    /// @tparam T The type to check for
    struct is_dyn_castable<T, std::void_t<decltype(T::classof_v), decltype(std::declval<T>().classof())>>
    {
      /// @brief True if dyn_cast can be used on T
      static constexpr bool value = std::is_same_v<typename std::decay_t<decltype(T::classof_v)>, typename std::decay_t<decltype(std::declval<T>().classof())>>;
    };

    template<typename T>
    /// @brief Short hand over is_dyn_castable<T>::value
    /// @tparam T The type to check for
    static constexpr bool is_dyn_castable_v = is_dyn_castable<T>::value;
  }  

  template<typename To, typename From>
  [[nodiscard]]
  /// @brief Try to cast 'ptr' from 'From*' to 'To*'.
  /// This is useful for safe down cast (casting from base to derived).
  /// Both 'To' and 'From' should be pointers.
  /// Both types should provide a 'static constexpr <TYPE> classof_v' constant,
  /// and a 'classof()' method which returns the same type as 'classof_v'.
  /// @tparam To The type to cast to
  /// @tparam From The type to cast from
  /// @param ptr The pointer to try to cast
  /// @return The casted pointer on success, else nullptr
  static constexpr std::enable_if_t<std::is_pointer_v<To>, To> dyn_cast(From ptr) noexcept
  {
    static_assert(std::is_pointer_v<From> && std::is_pointer_v<To>,
      "Both types should be pointers!");
    static_assert(!std::is_same_v<From, To>,
      "Remove this useless cast!");
    
    //Types of the pointers
    using To_t = typename std::pointer_traits<To>::element_type;
    using From_t = typename std::pointer_traits<From>::element_type;

    static_assert(traits::is_dyn_castable_v<To_t> && traits::is_dyn_castable_v<From_t>,
      "Both types should have a 'static constexpr ... classof_v = ...' and a 'classof()' method!");
    static_assert(std::is_same_v<decltype(To_t::classof_v), decltype(From_t::classof_v)>,
      "Type of 'classof_v' of To and From should match!");
    static_assert(To_t::classof_v != From_t::classof_v,
      "Value of To::classof_v and From::classof_v cannot be equal!");
    static_assert(std::is_same_v<decltype(std::declval<To_t>().classof()), decltype(std::declval<From_t>().classof())>,
      "Return type of 'classof()' of To and From should match!");
    
    if (ptr->classof() == To_t::classof_v)
      return static_cast<To>(ptr);
    return nullptr;
  }

  template<typename To, typename From>
  [[nodiscard]]
  /// @brief Try to cast 'ptr' from 'From*' to 'To*'.
  /// This is useful for safe down cast (casting from base to derived).
  /// Both 'To' and 'From' should be pointers.
  /// Both types should provide a 'static constexpr <TYPE> classof_v' constant,
  /// and a 'classof()' method which returns the same type as 'classof_v'.
  /// @tparam To The type to cast to
  /// @tparam From The type to cast from
  /// @param ptr The pointer to try to cast
  /// @return The casted pointer on success, else nullptr
  static constexpr std::enable_if_t<!std::is_pointer_v<To>, std::add_pointer_t<To>> dyn_cast(From ptr) noexcept
  {
    static_assert(std::is_pointer_v<From>,
      "'From' should be a pointer!");
    static_assert(!std::is_same_v<From, To>,
      "Remove this useless cast!");

    //Types of the pointers
    using To_t = To;
    using From_t = typename std::pointer_traits<From>::element_type;

    static_assert(traits::is_dyn_castable_v<To_t> && traits::is_dyn_castable_v<From_t>,
      "Both types should have a 'static constexpr ... classof_v = ...' and a 'classof()' method!");
    static_assert(std::is_same_v<decltype(To_t::classof_v), decltype(From_t::classof_v)>,
      "Type of 'classof_v' of To and From should match!");
    static_assert(To_t::classof_v != From_t::classof_v,
      "Value of To::classof_v and From::classof_v cannot be equal!");
    static_assert(std::is_same_v<decltype(std::declval<To_t>().classof()), decltype(std::declval<From_t>().classof())>,
      "Return type of 'classof()' of To and From should match!");

    if (ptr->classof() == To_t::classof_v)
      return static_cast<std::add_pointer_t<To>>(ptr);
    return nullptr;
  }

  template<typename To, typename From>
  [[nodiscard]]
  /// @brief Check if a pointer's real type is 'To'.
  /// Useful for getting the type of pointer in inheritances.
  /// Both 'To' and 'From' should be pointers.
  /// Both 'To' and 'From' should provide a 'static constexpr <TYPE> classof_v' constant,
  /// and a 'classof()' method which returns the same type as 'classof_v'.
  /// @tparam To The real type to check for
  /// @tparam From The type to check from
  /// @param ptr The pointer whose real type to compare with 'To'
  /// @return True if 'ptr' is a 'To' and can be safely casted to a 'To'
  static constexpr std::enable_if_t<std::is_pointer_v<To>, bool> is_a(From ptr) noexcept
  {
    static_assert(std::is_pointer_v<From> && std::is_pointer_v<To>,
      "Both types should be pointers!");
    static_assert(!std::is_same_v<From, To>,
      "Remove this useless cast!");

    //Types of the pointers
    using To_t = typename std::pointer_traits<To>::element_type;
    using From_t = typename std::pointer_traits<From>::element_type;

    static_assert(traits::is_dyn_castable_v<To_t> && traits::is_dyn_castable_v<From_t>,
      "Both types should have a 'static constexpr ... classof_v = ...' and a 'classof()' method!");
    static_assert(std::is_same_v<decltype(To_t::classof_v), decltype(From_t::classof_v)>,
      "Type of 'classof_v' of To and From should match!");
    static_assert(To_t::classof_v != From_t::classof_v,
      "Value of To::classof_v and From::classof_v cannot be equal!");
    static_assert(std::is_same_v<decltype(std::declval<To_t>().classof()), decltype(std::declval<From_t>().classof())>,
      "Return type of 'classof()' of To and From should match!");
    
    return ptr->classof() == To_t::classof_v;
  }

  template<typename To, typename From>
  [[nodiscard]]
  /// @brief Check if a pointer's real type is 'To'.
  /// Useful for getting the type of pointer in inheritances.
  /// Both 'To' and 'From' should be pointers.
  /// Both 'To' and 'From' should provide a 'static constexpr <TYPE> classof_v' constant,
  /// and a 'classof()' method which returns the same type as 'classof_v'.
  /// @tparam To The real type to check for
  /// @tparam From The type to check from
  /// @param ptr The pointer whose real type to compare with 'To'
  /// @return True if 'ptr' is a 'To' and can be safely casted to a 'To'
  static constexpr std::enable_if_t<!std::is_pointer_v<To>, bool> is_a(From ptr) noexcept
  {
    static_assert(std::is_pointer_v<From>,
      "'From' should be a pointer!");
    static_assert(!std::is_same_v<From, std::add_pointer_t<To>>,
      "Remove this useless cast!");

    //Types of the pointers
    using To_t = To;
    using From_t = typename std::pointer_traits<From>::element_type;

    static_assert(traits::is_dyn_castable_v<To_t> && traits::is_dyn_castable_v<From_t>,
      "Both types should have a 'static constexpr ... classof_v = ...' and a 'classof()' method!");
    static_assert(std::is_same_v<decltype(To_t::classof_v), decltype(From_t::classof_v)>,
      "Type of 'classof_v' of To and From should match!");
    static_assert(To_t::classof_v != From_t::classof_v,
      "Value of To::classof_v and From::classof_v cannot be equal!");
    static_assert(std::is_same_v<decltype(std::declval<To_t>().classof()), decltype(std::declval<From_t>().classof())>,
      "Return type of 'classof()' of To and From should match!");

    return ptr->classof() == To_t::classof_v;
  }

  template<typename Target, typename Input>
  [[nodiscard]]
  /// @brief Short-hand for static_cast<Target>(Input)
  /// @tparam Target The resulting type
  /// @tparam Input The type to convert from
  /// @param input The value to convert
  /// @return static_cast<Target>(input)
  static constexpr Target as(Input&& input)
  {    
#ifdef COLT_DEBUG_BUILD
    if constexpr (traits::is_dyn_castable_v<Input>
      && traits::is_dyn_castable_v<Target>
      && std::is_pointer_v<Target>
      && std::is_pointer_v<Input>)
    {
      if (is_a<Target>(input))
        return static_cast<Target>(std::forward<Input>(input));
      colt_unreachable("'as' conversion failed as true type did not match the expected one!");
    }
#endif
    return static_cast<Target>(std::forward<Input>(input));
  }
}

#endif //!HG_COLT_DYN_CAST