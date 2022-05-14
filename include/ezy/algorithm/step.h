#ifndef EZY_ALGORITHM_STEP_H_INCLUDED
#define EZY_ALGORITHM_STEP_H_INCLUDED

#include <ezy/range.h>

namespace ezy
{
  template <typename Range>
  constexpr auto step_by(Range&& range, detail::size_type_t<Range> n)
  {
    using ResultRange = detail::step_by_range_view<experimental::detail::deduce_keeper_t<Range>>;
    return ResultRange{
      ezy::experimental::make_keeper(std::forward<Range>(range)), n
    };
  }
}

#endif
