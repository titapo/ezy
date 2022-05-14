#ifndef EZY_ALGORITHM_REPEAT_H_INCLUDED
#define EZY_ALGORITHM_REPEAT_H_INCLUDED

#include <ezy/range.h>

namespace ezy
{
  template <typename T>
  constexpr auto repeat(T&& t)
  {
    return ezy::detail::repeat_view<T>{std::forward<T>(t)};
  }

}

#endif
