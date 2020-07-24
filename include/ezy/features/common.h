#ifndef EZY_FEATURES_COMMON_H_INCLUDED
#define EZY_FEATURES_COMMON_H_INCLUDED

#include "../feature.h"
#include "../strong_type_traits.h"

#include <cstddef>

namespace ezy
{
namespace features
{
  template <typename T>
  struct operator_arrow : feature<T, operator_arrow>
  {
    using base = feature<T, operator_arrow>;

    /* operator-> */
    constexpr decltype(auto) operator->() const
    { return (*this).underlying().operator->(); }

    constexpr decltype(auto) operator->()
    { return (*this).underlying().operator->(); }
  };

  template <typename T>
  struct operator_star : feature<T, operator_star>
  {
    using base = feature<T, operator_star>;

    /* operator* */
    constexpr decltype(auto) operator*() const &
    { return (*this).underlying().operator*(); }

    constexpr decltype(auto) operator*() &
    { return (*this).underlying().operator*(); }

    constexpr decltype(auto) operator*() &&
    { return std::move(*this).underlying().operator*(); }

    constexpr decltype(auto) operator*() const &&
    { return std::move(*this).underlying().operator*(); }
  };

  template <typename T>
  struct operator_subscript : feature<T, operator_subscript>
  {
    using base = feature<T, operator_subscript>;

    using size_type_local = size_t;// typename base::self_type::type::size_type;

    constexpr decltype(auto) operator[](size_type_local pos)
    { return base::underlying()[pos]; }

    constexpr decltype(auto) operator[](size_type_local pos) const
    { return base::underlying()[pos]; }
  };

  /**
   * features
   */

  // TODO specify order: a << b, b << a
  // TODO specify return type
  template <typename U>
  struct left_shiftable_with
  {
    template <typename T>
    struct impl : feature<T, impl>
    {
      friend decltype(auto) operator<<(U& lhs, const T& rhs)
      {
        return lhs << rhs.get();
      }
    };
  };

  template <typename T>
  struct clonable : feature<T, clonable>
  {
    using base = feature<T, clonable>;

    T clone() const
    {
      return {base::underlying()};
    }
  };

  template <typename T>
  struct implicit_convertible : feature<T, implicit_convertible>
  {
    using base = feature<T, implicit_convertible>;
    using underlying_type = extract_underlying_type_t<T>;

    operator const underlying_type&() const
    {
      return base::underlying();
    }

    operator underlying_type&()
    {
      return base::underlying;
    }
  };
}
}



#endif
