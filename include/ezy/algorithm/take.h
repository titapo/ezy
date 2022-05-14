#ifndef EZY_ALGORITHM_TAKE_H_INCLUDED
#define EZY_ALGORITHM_TAKE_H_INCLUDED

#include <ezy/range.h>

namespace ezy
{
  template <typename Range>
  constexpr auto take(Range&& range, detail::size_type_t<Range> n)
  {
    using ResultRangeType = detail::take_n_range_view<experimental::detail::deduce_keeper_t<Range>>;
    return ResultRangeType{
      ezy::experimental::make_keeper(std::forward<Range>(range)),
      n
    };
  }

  template <typename Range, typename Predicate>
  /*constexpr*/ auto take_while(Range&& range, Predicate&& pred)
  {
    using ResultRangeType = detail::take_while_range_view<experimental::detail::deduce_keeper_t<Range>, ezy::remove_cvref_t<Predicate>>;
    return ResultRangeType{
      ezy::experimental::make_keeper(std::forward<Range>(range)),
      std::forward<Predicate>(pred)
    };
  }
}

#endif
