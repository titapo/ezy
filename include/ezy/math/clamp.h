#ifndef EZY_MATH_CLAMP_H_INCLUDED
#define EZY_MATH_CLAMP_H_INCLUDED

#include <algorithm>

namespace ezy
{
  struct clamp_fn
  {
    // returns by-value and not by reference: this is safer, but can suffer any performance or practical issue?
    template <typename T>
    constexpr auto operator()(const T& low, const T& high) const
    {
      return [low, high](const T& value)
      {
        return std::clamp(value, low, high);
      };
    }
  };

  inline namespace cpo
  {
    constexpr clamp_fn clamp{};
  }
}

#endif
