#ifndef EZY_MATH_MAX_H_INCLUDED
#define EZY_MATH_MAX_H_INCLUDED

#include <algorithm>

namespace ezy
{
  struct max_fn
  {
    template <typename T> 
    constexpr decltype(auto) operator()(const T& a, const T& b) const
    {
      return std::max(a, b);
    }

    template <typename TL> 
    constexpr decltype(auto) operator()(std::initializer_list<TL> tl) const
    {
      return std::max(tl);
    }
  };

  inline namespace cpo
  {
    constexpr max_fn max{};
  }

}

#endif
