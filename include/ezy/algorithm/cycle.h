#ifndef EZY_ALGORITHM_CYCLE_H_INCLUDED
#define EZY_ALGORITHM_CYCLE_H_INCLUDED

#include <ezy/range.h>

namespace ezy
{
  template <typename Range>
  constexpr auto cycle(Range&& range)
  {
    return detail::cycle_view<experimental::detail::deduce_keeper_t<Range>>{
      ezy::experimental::make_keeper(std::forward<Range>(range))
    };
  }
}

#endif
