#ifndef EZY_ALGORITHM_SPLIT_H_INCLUDED
#define EZY_ALGORITHM_SPLIT_H_INCLUDED

#include <ezy/range.h>

namespace ezy
{
  template <typename Range, typename Delimiter>
  auto split(Range&& range, Delimiter&& delimiter)
  {
    using ResultRange = detail::split_range_view<experimental::detail::deduce_keeper_t<Range>, experimental::detail::deduce_keeper_t<Delimiter>>;
    return ResultRange{
      ezy::experimental::make_keeper(std::forward<Range>(range)),
      ezy::experimental::make_keeper(std::forward<Delimiter>(delimiter))
    };
  }
}

#endif
