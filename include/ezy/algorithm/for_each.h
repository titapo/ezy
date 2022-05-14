#ifndef EZY_ALGORITHM_FOR_EACH_H_INCLUDED
#define EZY_ALGORITHM_FOR_EACH_H_INCLUDED

#include <algorithm>

namespace ezy
{
  template <typename Range, typename UnaryFunction>
  /*constexpr?*/ decltype(auto) for_each(Range&& range, UnaryFunction&& fn)
  {
    using std::begin;
    using std::end;
    return std::for_each(begin(range), end(range), std::forward<UnaryFunction>(fn));
  }
}

#endif
