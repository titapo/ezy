#ifndef EZY_ALGORITHM_ANY_OF_H_INCLUDED
#define EZY_ALGORITHM_ANY_OF_H_INCLUDED

#include <algorithm>

namespace ezy
{
  template <typename Range, typename Predicate>
  constexpr bool any_of(Range&& range, Predicate&& predicate)
  {
    return std::any_of(std::begin(range), std::end(range), std::forward<Predicate>(predicate));
  }
}

#endif
