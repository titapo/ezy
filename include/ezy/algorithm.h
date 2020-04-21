#ifndef EZY_ALGORITHM_H_INCLUDED
#define EZY_ALGORITHM_H_INCLUDED

#include "experimental/keeper.h"
#include "range.h"

#include "bits_empty_size.h"


namespace ezy
{
  namespace detail
  {
    template <typename Range>
    struct deducer_helper
    {
      using orig_type = Range;
      using underlying_range_type = std::remove_reference_t<orig_type>;
      using category_tag = ezy::experimental::detail::ownership_category_t<orig_type>;
    };
  }

  template <typename Range, typename UnaryFunction>
  /*constexpr?*/ decltype(auto) for_each(Range&& range, UnaryFunction&& fn)
  {
    using std::begin;
    using std::end;
    return std::for_each(begin(range), end(range), std::forward<UnaryFunction>(fn));
  }

  template <typename Range, typename UnaryFunction>
  constexpr decltype(auto) transform(Range&& range, UnaryFunction&& fn)
  {
    using range_type = typename detail::deducer_helper<Range>::underlying_range_type;
    using CategoryTag = typename detail::deducer_helper<Range>::category_tag;
    using result_range_type = range_view<CategoryTag, range_type, UnaryFunction>;
    return result_range_type{
        ezy::experimental::basic_keeper<CategoryTag, range_type>{std::forward<Range>(range)},
        std::forward<UnaryFunction>(fn)
      };
  }

  template <typename Range, typename Predicate>
  /*constexpr*/ auto filter(Range&& range, Predicate&& pred)
  {
    using range_type = typename detail::deducer_helper<Range>::underlying_range_type;
    using CategoryTag = typename detail::deducer_helper<Range>::category_tag;
    using result_range_type = range_view_filter<CategoryTag, range_type, Predicate>;
    return result_range_type{
      ezy::experimental::basic_keeper<CategoryTag, range_type>{std::forward<Range>(range)},
      std::forward<Predicate>(pred)
    };
  }

}

#endif
