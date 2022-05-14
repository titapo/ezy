#ifndef EZY_ALGORITHM_FLATTEN_H_INCLUDED
#define EZY_ALGORITHM_FLATTEN_H_INCLUDED

#include <ezy/range.h>

namespace ezy
{
  template <typename Range>
  /*constexpr*/ auto flatten(Range&& range)
  {
    using ResultRangeType = detail::flattened_range_view<experimental::detail::deduce_keeper_t<Range>>;
    return ResultRangeType{
      ezy::experimental::make_keeper(std::forward<Range>(range))
    };
  }
}

#endif
