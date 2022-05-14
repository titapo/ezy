#ifndef EZY_ALGORITHM_ZIP_H_INCLUDED
#define EZY_ALGORITHM_ZIP_H_INCLUDED

#include <ezy/bits/tuple.h>
#include <ezy/range.h>

namespace ezy {
  template <typename... Ranges>
  constexpr auto zip(Ranges&&... ranges)
  {
    using ResultRangeType = detail::zip_range_view<make_tuple_fn, experimental::detail::deduce_keeper_t<Ranges>... >;
    return ResultRangeType{
      make_tuple,
      ezy::experimental::make_keeper(std::forward<Ranges>(ranges))...
    };
  }

  template <typename Zipper, typename... Ranges>
  /*constexpr*/ auto zip_with(Zipper&& zipper, Ranges&&... ranges)
  {
    using ResultRangeType = detail::zip_range_view<Zipper, typename experimental::detail::deduce_keeper_t<Ranges>... >;
    return ResultRangeType{
      std::forward<Zipper>(zipper),
      ezy::experimental::make_keeper(std::forward<Ranges>(ranges))...
    };
  }


}

#endif
