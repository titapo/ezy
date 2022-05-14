#ifndef EZY_ALGORITHM_NONE_OF_H_INCLUDED
#define EZY_ALGORITHM_NONE_OF_H_INCLUDED

#include <algorithm>

namespace ezy
{
  template <typename Range, typename Predicate>
  constexpr bool none_of(Range&& range, Predicate&& predicate)
  {
    return std::none_of(std::begin(range), std::end(range), std::forward<Predicate>(predicate));
  }
}

#endif
