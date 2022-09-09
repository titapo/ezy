#ifndef EZY_ALGORITHM_SLICE_H_INCLUDED
#define EZY_ALGORITHM_SLICE_H_INCLUDED

#include <ezy/range.h>

namespace ezy
{
  template <typename Range>
  constexpr auto slice(Range&& range, detail::size_type_t<Range> from, detail::size_type_t<Range> until)
  {
    using ResultRangeType = detail::range_view_slice<experimental::detail::deduce_keeper_t<Range>>;
    return ResultRangeType{
      ezy::experimental::make_keeper(std::forward<Range>(range)),
      from,
      until
    };
  }
}

#endif
