#ifndef EZY_MATH_MIN_H_INCLUDED
#define EZY_MATH_MIN_H_INCLUDED

#include <algorithm>

namespace ezy
{
  struct min_fn
  {
    template <typename T> 
    constexpr decltype(auto) operator()(const T& a, const T& b) const
    {
      return std::min(a, b);
    }

    template <typename TL> 
    constexpr decltype(auto) operator()(std::initializer_list<TL> tl) const
    {
      return std::min(tl);
    }
  };

  inline namespace cpo
  {
    constexpr min_fn min{};
  }

}

#endif
