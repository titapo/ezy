#ifndef EZY_ALGORITHM_CONTAINS_H_INCLUDED
#define EZY_ALGORITHM_CONTAINS_H_INCLUDED

#include <ezy/algorithm/find_element.h>

namespace ezy
{

  template <typename Range, typename Needle>
  constexpr bool contains(Range&& range, Needle&& needle)
  {
    return find_element(range, needle) != end(range);
  }
}

#endif
