#ifndef EZY_ALGORITHM_CHUNK_H_INCLUDED
#define EZY_ALGORITHM_CHUNK_H_INCLUDED

#include <ezy/range.h>

namespace ezy
{
  template <typename Range>
  constexpr auto chunk(Range&& range, size_t chunk_size)
  {
    using ResultRange = detail::chunk_range_view<experimental::detail::deduce_keeper_t<Range>>;
    return ResultRange{ezy::experimental::make_keeper(std::forward<Range>(range)), chunk_size};
  }
}

#endif
