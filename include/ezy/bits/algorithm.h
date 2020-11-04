#ifndef EZY_BITS_ALGORITHM_H_INCLUDED
#define EZY_BITS_ALGORITHM_H_INCLUDED

#include <ezy/experimental/keeper.h>
#include <ezy/range.h>

#include "empty_size.h"

#include <numeric> // accumulate
#include <algorithm>


namespace ezy
{
  namespace detail
  {
    template <typename Range>
    using deduce_keeper_t = ezy::experimental::keeper<
      ezy::experimental::detail::ownership_category_t<Range>,
      std::remove_reference_t<Range>
    >;
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
    using result_range_type = detail::range_view<detail::deduce_keeper_t<Range>, UnaryFunction>;
    return result_range_type{
        ezy::experimental::make_keeper(std::forward<Range>(range)),
        std::forward<UnaryFunction>(fn)
      };
  }

  template <typename Range, typename Predicate>
  /*constexpr*/ auto filter(Range&& range, Predicate&& pred)
  {
    using result_range_type = detail::range_view_filter<detail::deduce_keeper_t<Range>, Predicate>;
    return result_range_type{
      ezy::experimental::make_keeper(std::forward<Range>(range)),
      std::forward<Predicate>(pred)
    };
  }

  template <typename Range1, typename Range2>
  /*constexpr*/ auto concatenate(Range1&& range1, Range2&& range2)
  {

    using ResultRangeType = detail::concatenated_range_view<
      detail::deduce_keeper_t<Range1>,
      detail::deduce_keeper_t<Range2>
      >;

    return ResultRangeType{
      ezy::experimental::make_keeper(std::forward<Range1>(range1)),
      ezy::experimental::make_keeper(std::forward<Range2>(range2))
    };
  }

  struct make_tuple_fn
  {
    template <typename... Args>
    constexpr auto operator()(Args&&... args) const
    {
      return std::make_tuple(std::forward<Args>(args)...);
    }
  };

  inline constexpr make_tuple_fn make_tuple{};

  struct forward_as_tuple_fn
  {
    template <typename... Args>
    constexpr auto operator()(Args&&... args) const
    {
      return std::forward_as_tuple(std::forward<decltype(args)>(args)...);
    }
  };

  inline constexpr forward_as_tuple_fn forward_as_tuple{};


  template <typename... Ranges>
  constexpr auto zip(Ranges&&... ranges)
  {
    using ResultRangeType = detail::zip_range_view<make_tuple_fn, detail::deduce_keeper_t<Ranges>... >;
    return ResultRangeType{
      make_tuple,
      ezy::experimental::make_keeper(std::forward<Ranges>(ranges))...
    };
  }

  template <typename Zipper, typename... Ranges>
  /*constexpr*/ auto zip_with(Zipper&& zipper, Ranges&&... ranges)
  {
    using ResultRangeType = detail::zip_range_view<Zipper, typename detail::deduce_keeper_t<Ranges>... >;
    return ResultRangeType{
      std::forward<Zipper>(zipper),
      ezy::experimental::make_keeper(std::forward<Ranges>(ranges))...
    };
  }

  template <typename Range>
  /*constexpr*/ auto slice(Range&& range, unsigned int from, unsigned int until) // TODO check size_type
  {
    using ResultRangeType = detail::range_view_slice<detail::deduce_keeper_t<Range>>;
    return ResultRangeType{
      ezy::experimental::make_keeper(std::forward<Range>(range)),
      from,
      until
    };
  }

  template <typename Range>
  /*constexpr*/ auto take(Range&& range, size_t n) // TODO check size_type
  {
    using ResultRangeType = detail::take_n_range_view<detail::deduce_keeper_t<Range>>;
    return ResultRangeType{
      ezy::experimental::make_keeper(std::forward<Range>(range)),
      n
    };
  }

  template <typename Range, typename Predicate>
  /*constexpr*/ auto take_while(Range&& range, Predicate&& pred)
  {
    using ResultRangeType = detail::take_while_range_view<detail::deduce_keeper_t<Range>, ezy::remove_cvref_t<Predicate>>;
    return ResultRangeType{
      ezy::experimental::make_keeper(std::forward<Range>(range)),
      std::forward<Predicate>(pred)
    };
  }

  template <typename Range>
  constexpr auto drop(Range&& range, size_t n)
  {
    using ResultRangeType = detail::drop_range_view<detail::deduce_keeper_t<Range>>;
    return ResultRangeType{
      ezy::experimental::make_keeper(std::forward<Range>(range)), n
    };
  }

  template <typename Range>
  constexpr auto step_by(Range&& range, size_t n)
  {
    using ResultRange = detail::step_by_range_view<detail::deduce_keeper_t<Range>>;
    return ResultRange{
      ezy::experimental::make_keeper(std::forward<Range>(range)), n
    };
  }

  template <typename Range>
  /*constexpr*/ auto flatten(Range&& range)
  {
    using ResultRangeType = detail::flattened_range_view<detail::deduce_keeper_t<Range>>;
    return ResultRangeType{
      ezy::experimental::make_keeper(std::forward<Range>(range))
    };
  }

  template <typename Range, typename Predicate>
  constexpr bool all_of(Range&& range, Predicate&& predicate)
  {
    return std::all_of(std::begin(range), std::end(range), std::forward<Predicate>(predicate));
  }

  template <typename Range, typename Predicate>
  constexpr bool any_of(Range&& range, Predicate&& predicate)
  {
    return std::any_of(std::begin(range), std::end(range), std::forward<Predicate>(predicate));
  }

  template <typename Range, typename Predicate>
  constexpr bool none_of(Range&& range, Predicate&& predicate)
  {
    return std::none_of(std::begin(range), std::end(range), std::forward<Predicate>(predicate));
  }

  namespace detail
  {
    template <typename Range, typename Needle>
    constexpr auto find_element_impl(Range&& range, Needle&& needle, priority_tag<0>)
    {
      using std::begin;
      using std::end;
      return std::find(begin(range), end(range), std::forward<Needle>(needle));
    }

    template <typename Range, typename Needle>
    constexpr auto find_element_impl(Range&& range, Needle&& needle, priority_tag<1>)
      -> decltype(range.find(std::forward<Needle>(needle)))
    {
      return range.find(std::forward<Needle>(needle));
    }
  }

  template <typename Range, typename Needle>
  constexpr auto find_element(Range&& range, Needle&& needle)
  {
    static_assert(std::is_same_v<
        ezy::experimental::detail::ownership_category_t<Range>,
        ezy::experimental::reference_category_tag
        >, "Range must be a reference! Cannot form an iterator to a temporary!");

    return detail::find_element_impl(std::forward<Range>(range), std::forward<Needle>(needle), detail::priority_tag<1>{});
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

  template <typename Range, typename Needle>
  constexpr bool contains(Range&& range, Needle&& needle)
  {
    return find_element(range, needle) != end(range);
  }

  template <typename Result, typename Range>
  constexpr auto collect(Range&& range)
  {
    using std::cbegin;
    using std::cend;
    return Result(cbegin(range), cend(range));
  }

  template <template <typename, typename ...> class ResultWrapper, typename Range>
  constexpr auto collect(Range&& range)
  {
    using std::begin;
    using ElementType = ezy::remove_cvref_t<decltype(*begin(range))>;
    return collect<ResultWrapper<ElementType>>(std::forward<Range>(range));
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

  template <typename T, typename Fn>
  constexpr auto iterate(T&& t, Fn&& fn)
  {
    return ezy::detail::iterate_view<T, Fn>{std::forward<T>(t), std::forward<Fn>(fn)};
  }

  namespace detail
  {
    struct pre_increment
    {
      template <typename T>
      constexpr decltype(auto) operator()(T&& t) const
      {
        return ++t;
      }
    };
  }

  template <typename T>
  constexpr auto iterate(T&& t)
  {
    using Fn = detail::pre_increment;
    return ezy::detail::iterate_view<T, Fn>{std::forward<T>(t), Fn{}};
  }

  template <typename Range>
  constexpr auto enumerate(Range&& range)
  {
    using SizeType = detail::size_type_t<Range>;
    return ezy::zip(ezy::iterate<SizeType>({}), std::forward<Range>(range));
  }

  template <typename Range>
  constexpr auto cycle(Range&& range)
  {
    return detail::cycle_view<detail::deduce_keeper_t<Range>>{
      ezy::experimental::make_keeper(std::forward<Range>(range))
    };
  }

  template <typename T>
  constexpr auto repeat(T&& t)
  {
    return ezy::detail::repeat_view<T>{std::forward<T>(t)};
  }

  namespace detail
  {
    // accumulate polyfill brings c++20 (constexpr, move) features to c++14
    template <typename ItFirst, typename ItLast, typename T>
    constexpr T accumulate(ItFirst first, ItLast last, T init)
    {
      for (; first != last; ++first)
      {
        init = std::move(init) + *first;
      }
      return init;
    }

    template <typename ItFirst, typename ItLast, typename T, typename BinaryOp>
    constexpr T accumulate(ItFirst first, ItLast last, T init, BinaryOp op)
    {
      for (; first != last; ++first)
      {
        init = ezy::invoke(op, std::move(init), *first);
      }
      return init;
    }
  }

  template <typename Range, typename Init>
  constexpr Init accumulate(Range&& range, Init&& init)
  {
    return detail::accumulate(std::begin(range), std::end(range), std::forward<Init>(init));
  }

  template <typename Range, typename Init, typename BinaryOp>
  constexpr Init accumulate(Range&& range, Init&& init, BinaryOp&& op)
  {
    return detail::accumulate(std::begin(range), std::end(range), std::forward<Init>(init), std::forward<BinaryOp>(op));
  }

  template <typename Range>
  constexpr auto chunk(Range&& range, size_t chunk_size)
  {
    using ResultRange = detail::chunk_range_view<detail::deduce_keeper_t<Range>>;
    return ResultRange{ezy::experimental::make_keeper(std::forward<Range>(range)), chunk_size};
  }
}

#endif
