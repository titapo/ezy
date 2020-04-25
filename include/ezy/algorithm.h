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

  template <typename Range1, typename Range2>
  /*constexpr*/ auto concatenate(Range1&& range1, Range2&& range2)
  {
    using Range1Type = typename detail::deducer_helper<Range1>::underlying_range_type;
    using Category1Tag = typename detail::deducer_helper<Range1>::category_tag;

    using Range2Type = typename detail::deducer_helper<Range2>::underlying_range_type;
    using Category2Tag = typename detail::deducer_helper<Range2>::category_tag;

    using ResultRangeType = concatenated_range_view<Category1Tag, Range1Type, Category2Tag, Range2Type>;
    return ResultRangeType{
        ezy::experimental::basic_keeper<Category1Tag, Range1Type>(std::forward<Range1>(range1)),
        ezy::experimental::basic_keeper<Category2Tag, Range2Type>(std::forward<Range2>(range2))
    };
  }

  template <typename Range1, typename Range2>
  /*constexpr*/ auto zip(Range1&& range1, Range2&& range2)
  {
    using Range1Type = typename detail::deducer_helper<Range1>::underlying_range_type;
    using Category1Tag = typename detail::deducer_helper<Range1>::category_tag;

    using Range2Type = typename detail::deducer_helper<Range2>::underlying_range_type;
    using Category2Tag = typename detail::deducer_helper<Range2>::category_tag;

    using ResultRangeType = zip_range_view<Category1Tag, Range1Type, Category2Tag, Range2Type>;
    return ResultRangeType{
      ezy::experimental::basic_keeper<Category1Tag, Range1Type>(std::forward<Range1>(range1)),
      ezy::experimental::basic_keeper<Category2Tag, Range2Type>(std::forward<Range2>(range2))
    };
  }

  template <typename Range>
  /*constexpr*/ auto slice(Range&& range, unsigned int from, unsigned int until) // TODO check size_type
  {
    using RangeType = typename detail::deducer_helper<Range>::underlying_range_type;
    using CategoryTag = typename detail::deducer_helper<Range>::category_tag;
    using ResultRangeType = range_view_slice<CategoryTag, RangeType>;
    return ResultRangeType{
      ezy::experimental::basic_keeper<CategoryTag, RangeType>(std::forward<Range>(range)),
      from,
      until
    };
  }

  template <typename Range>
  /*constexpr*/ auto take(Range&& range, size_t n) // TODO check size_type
  {
    using RangeType = typename detail::deducer_helper<Range>::underlying_range_type;
    using CategoryTag = typename detail::deducer_helper<Range>::category_tag;
    using ResultRangeType = take_n_range_view<CategoryTag, RangeType>;
    return ResultRangeType{
      ezy::experimental::basic_keeper<CategoryTag, RangeType>(std::forward<Range>(range)),
      n
    };
  }

  template <typename Range, typename Predicate>
  /*constexpr*/ auto take_while(Range&& range, Predicate&& pred)
  {
    using RangeType = typename detail::deducer_helper<Range>::underlying_range_type;
    using CategoryTag = typename detail::deducer_helper<Range>::category_tag;
    using ResultRangeType = take_while_range_view<CategoryTag, RangeType, ezy::remove_cvref_t<Predicate>>;
    return ResultRangeType{
      ezy::experimental::basic_keeper<CategoryTag, RangeType>(std::forward<Range>(range)),
      std::forward<Predicate>(pred)
    };
  }

  template <typename Range>
  /*constexpr*/ auto flatten(Range&& range)
  {
    using RangeType = typename detail::deducer_helper<Range>::underlying_range_type;
    using CategoryTag = typename detail::deducer_helper<Range>::category_tag;
    using ResultRangeType = flattened_range_view<CategoryTag, RangeType>;
    return ResultRangeType{
      ezy::experimental::basic_keeper<CategoryTag, RangeType>(std::forward<Range>(range))
    };
  }


}

#endif
