#ifndef EZY_ALGORITHM_DROP_H_INCLUDED
#define EZY_ALGORITHM_DROP_H_INCLUDED

#include <ezy/range.h>

namespace ezy
{
  template <typename Range>
  constexpr auto drop(Range&& range, detail::size_type_t<Range> n)
  {
    using ResultRangeType = detail::drop_range_view<experimental::detail::deduce_keeper_t<Range>>;
    return ResultRangeType{
      ezy::experimental::make_keeper(std::forward<Range>(range)), n
    };
  }
}

#endif
