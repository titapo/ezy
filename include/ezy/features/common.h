#ifndef EZY_FEATURES_COMMON_H_INCLUDED
#define EZY_FEATURES_COMMON_H_INCLUDED

#include "../strong_type.h"

namespace ezy::features
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
}


#endif
