#ifndef EZY_ALGORITHM_H_INCLUDED
#define EZY_ALGORITHM_H_INCLUDED

#include "experimental/keeper.h"
#include "range.h"
#include "optional" // ezy::optional for find
#include "pointer.h" // for find

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

  namespace detail
  {
    template <typename Range, typename Key>
    using find_mem_fn_t = decltype(std::declval<Range>().find(std::declval<Key>()));
  }

  template <typename Range, typename Needle>
  /*constexpr*/ auto find_element(Range&& range, Needle&& needle)
  {
    static_assert(std::is_same_v<
        ezy::experimental::detail::ownership_category_t<Range>,
        ezy::experimental::reference_category_tag
        >, "Range must be a reference! Cannot form an iterator to a temporary!");

    using std::begin;
    using std::end;
    if constexpr (std::experimental::is_detected<detail::find_mem_fn_t, Range, Needle>::value)
    {
      return range.find(std::forward<Needle>(needle));
    }
    else
    {
      return std::find(begin(range), end(range), std::forward<Needle>(needle));
    }
  }

  namespace detail
  {
    template <typename T>
    decltype(auto) dependent_forward_impl(T&& t, ezy::experimental::owner_category_tag, std::false_type)
    { return std::move(t); }

    template <typename T>
    decltype(auto) dependent_forward_impl(T&& t, ezy::experimental::reference_category_tag, std::false_type)
    { return &t; }

    // TODO const cases are missing

    template <typename Dependence, typename T>
    decltype(auto) dependent_forward(T&& t)
    {
      return dependent_forward_impl(
          std::forward<T>(t),
          ezy::experimental::detail::ownership_category_t<Dependence>{},
          std::is_const<Dependence>{}
      );
    }
  }

  template <typename Range, typename Needle>
  /*constexpr*/ auto find(Range&& range, Needle&& needle)
  {
    using ValueType = std::remove_reference_t<decltype(*begin(range))>;
    using result_type = std::conditional_t<
      std::is_lvalue_reference_v<Range>,
      const ezy::pointer<ValueType>,
      ezy::optional<ValueType>
    >;

    auto found = find_element(range, std::forward<Needle>(needle));
    if (found != end(range))
      return result_type(detail::dependent_forward<Range>(*found));
    else
      return result_type();
  }

  template <typename Range, typename Predicate>
  /*constexpr*/ auto find_element_if(Range&& range, Predicate&& pred)
  {
    static_assert(std::is_same_v<
        ezy::experimental::detail::ownership_category_t<Range>,
        ezy::experimental::reference_category_tag
        >, "Range must be a reference! Cannot form an iterator to a temporary!");

    using std::begin;
    using std::end;
    return std::find_if(begin(range), end(range), std::forward<Predicate>(pred));
  }

  template <typename Range, typename Predicate>
  /*constexpr*/ auto find_if(Range&& range, Predicate&& pred)
  {
    using ValueType = std::remove_reference_t<decltype(*begin(range))>;
    using result_type = std::conditional_t<
      std::is_lvalue_reference_v<Range>,
      const ezy::pointer<ValueType>,
      ezy::optional<ValueType>
    >;

    const auto found = find_element_if(range, std::forward<Predicate>(pred));
    if (found != end(range))
      return result_type(detail::dependent_forward<Range>(*found));
    else
      return result_type();
  }

  template <typename Result, typename Range>
  constexpr auto collect(Range&& range)
  {
    using std::cbegin;
    using std::cend;
    return Result(cbegin(range), cend(range));
  }

  template <template <typename> class ResultWrapper, typename Range>
  constexpr auto collect(Range&& range)
  {
    using std::begin;
    using ElementType = std::remove_reference_t<decltype(*begin(range))>;
    return collect<ResultWrapper<ElementType>>(std::forward<Range>(range));
  }

  namespace detail
  {
     template <typename T>
     struct value_type
     {
       using type = ezy::remove_cvref_t<decltype(*std::begin(std::declval<T>()))>;
     };

     template <typename T>
     using value_type_t = typename value_type<T>::type;
  }

  template <typename ReturnType, typename Range, typename Separator = detail::value_type_t<Range>>
  constexpr ReturnType join(Range&& range, Separator&& separator = Separator{})
  {
    using std::begin;
    using std::end;
    if (ezy::empty(range))
    {
      return ReturnType{};
    }
    else
    {
      ReturnType result{*begin(range)};
      std::for_each(
          ++begin(range), // std::next() would be better
          end(range),
          [&result,separator](const auto& e)
          {
            result += separator;
            result += e;
          }
        );
      return result;
    }
  }

  template <typename Range, typename Separator = detail::value_type_t<Range>>
  constexpr auto join(Range&& range, Separator&& separator = Separator{})
  {
    using std::begin;
    using std::end;
    using ValueType = detail::value_type_t<Range>;
    return join<ValueType>(std::forward<Range>(range), std::forward<Separator>(separator));
  }


}

#endif
