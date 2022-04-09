#ifndef EZY_MATH_ABS_H_INCLUDED
#define EZY_MATH_ABS_H_INCLUDED

#include <cstdlib>

namespace ezy
{
  struct abs_fn
  {
    template <typename T>
    constexpr auto operator()(T t) const
    {
      return std::abs(t);
    }
  };

  inline namespace cpo
  {
    constexpr abs_fn abs{};
  }

}

#endif
