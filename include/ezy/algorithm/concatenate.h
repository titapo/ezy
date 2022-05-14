#ifndef EZY_ALGORITHM_CONCATENATE_H_INCLUDED
#define EZY_ALGORITHM_CONCATENATE_H_INCLUDED

#include <ezy/range.h>

namespace ezy
{
  template <typename Range1, typename Range2>
  /*constexpr*/ auto concatenate(Range1&& range1, Range2&& range2)
  {
    using ResultRangeType = detail::concatenated_range_view<
      experimental::detail::deduce_keeper_t<Range1>,
      experimental::detail::deduce_keeper_t<Range2>
      >;

    return ResultRangeType{
      ezy::experimental::make_keeper(std::forward<Range1>(range1)),
      ezy::experimental::make_keeper(std::forward<Range2>(range2))
    };
  }
}

#endif
