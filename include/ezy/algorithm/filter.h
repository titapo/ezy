#ifndef EZY_ALGORITHM_FILTER_H_INCLUDED
#define EZY_ALGORITHM_FILTER_H_INCLUDED

#include <ezy/range.h>

namespace ezy
{
  template <typename Range, typename Predicate>
  /*constexpr*/ auto filter(Range&& range, Predicate&& pred)
  {
    using result_range_type = detail::range_view_filter<experimental::detail::deduce_keeper_t<Range>, Predicate>;
    return result_range_type{
      ezy::experimental::make_keeper(std::forward<Range>(range)),
      std::forward<Predicate>(pred)
    };
  }

}

#endif
