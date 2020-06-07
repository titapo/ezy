#ifndef EZY_FEATURES_ITERABLE_H_INCLUDED
#define EZY_FEATURES_ITERABLE_H_INCLUDED

#include <algorithm>
#include <functional>
#include <numeric> // for accumulate and reduce

#include "../range.h"
#include "../optional" // for ezy::optional
#include "../algorithm.h"

#include <experimental/type_traits>

namespace ezy::features
{
  template <typename T>
  struct has_iterator : feature<T, has_iterator>
  {
    using base = feature<T, has_iterator>;

    constexpr auto begin() { return std::begin(base::underlying()); }
    constexpr auto begin() const { return std::begin(base::underlying()); }
    constexpr auto cbegin() { return std::cbegin(base::underlying()); }
    constexpr auto cbegin() const { return std::cbegin(base::underlying()); }

    constexpr auto end() { return std::end(base::underlying()); }
    constexpr auto end() const { return std::end(base::underlying()); }
    constexpr auto cend() { return std::cend(base::underlying()); }
    constexpr auto cend() const { return std::cend(base::underlying()); }
  };

  template <typename T>
  struct inherit_range_view_features : crtp<T, inherit_range_view_features>
  {
    using const_iterator = typename T::const_iterator;
  };

  template <typename T>
  struct algo_iterable : feature<T, algo_iterable>
  {
    using base = feature<T, algo_iterable>;

    template < typename UnaryFunction >
    auto for_each(UnaryFunction f)
    {
      return ezy::for_each((*this).self(), std::forward<UnaryFunction>(f));
    }

    template <typename UnaryFunction>
    auto for_each(UnaryFunction f) const
    {
      return ezy::for_each((*this).self(), std::forward<UnaryFunction>(f));
    }

    template <typename UnaryFunction>
    constexpr auto map(UnaryFunction&& f) const &
    {
      return ezy::make_strong<ezy::notag_t, has_iterator, algo_iterable>(
          ezy::transform((*this).underlying(), std::forward<UnaryFunction>(f))
        );
    }

    template <typename UnaryFunction>
    constexpr auto map(UnaryFunction&& f) &&
    {
      return ezy::make_strong<ezy::notag_t, has_iterator, algo_iterable>(
          ezy::transform(std::move(*this).underlying(), std::forward<UnaryFunction>(f))
        );
    }

    template <typename UnaryFunction>
    auto flat_map(UnaryFunction&& f) const &
    {
      return (*this).flatten().map(std::forward<UnaryFunction>(f));
    }

    template <typename UnaryFunction>
    auto flat_map(UnaryFunction&& f) &&
    {
      return std::move(*this).flatten().map(std::forward<UnaryFunction>(f));
    }

    template <typename RhsRange>
    auto concatenate(RhsRange&& rhs) const &
    {
      return ezy::make_strong<ezy::notag_t, has_iterator, algo_iterable>(
          ezy::concatenate((*this).underlying(), std::forward<RhsRange>(rhs))
          );
    }

    template <typename RhsRange>
    auto concatenate(RhsRange&& rhs) &&
    {
      return ezy::make_strong<ezy::notag_t, has_iterator, algo_iterable>(
          ezy::concatenate(std::move(*this).underlying(), std::forward<RhsRange>(rhs))
          );
    }

    template <typename Predicate>
    auto filter(Predicate&& predicate) const &
    {
      return ezy::make_strong<ezy::notag_t, has_iterator, algo_iterable>(
          ezy::filter((*this).underlying(), std::forward<Predicate>(predicate))
        );
    }

    template <typename Predicate>
    auto filter(Predicate&& predicate) &&
    {
      return ezy::make_strong<ezy::notag_t, has_iterator, algo_iterable>(
          ezy::filter(std::move(*this).underlying(), std::forward<Predicate>(predicate))
        );
    }

    template <typename Element>
    auto find(Element&& element) const
    {
      return ezy::find((*this).underlying(), std::forward<Element>(element));
    }

    template <typename Predicate>
    auto find_if(Predicate&& predicate) const
    {
      using range_type = typename std::remove_reference<typename T::type>::type;
      using result_type = ezy::optional<typename range_type::value_type>;
      const auto found = std::find_if(base::underlying().begin(), base::underlying().end(), std::forward<Predicate>(predicate));
      if (found != base::underlying().end())
        return result_type(*found);
      else
        return result_type();
    }

    [[nodiscard]] constexpr auto empty() const
    {
      return ezy::empty(base::underlying());
    }

    [[nodiscard]] constexpr auto size() const
    {
      return ezy::size(base::underlying());
    }

    template <typename Element>
    bool contains(Element&& needle) const
    {
      return ezy::contains((*this).underlying(), std::forward<Element>(needle));
    }

    template <typename Type>
    Type accumulate(Type init) 
    {
      return std::accumulate(base::underlying().begin(), base::underlying().end(), init);
    }

    template <typename Type, typename BinaryOp>
    Type accumulate(Type init, BinaryOp op) 
    {
      return std::accumulate(base::underlying().begin(), base::underlying().end(), init, op);
    }

    /*
     * not found in gcc even if numeric has been included
    template <typename Type>
    Type reduce(Type init) 
    {
      return std::reduce(base::underlying().begin(), base::underlying().end(), init);
    }
    */

    template <typename Predicate>
    auto partition(Predicate&& predicate) const &
    {
      using range_type = typename std::remove_reference<typename T::type>::type;

      const auto negate_result = [](const Predicate& original_predicate)
      {
        return [=](const typename range_type::value_type& v)
        { return !original_predicate(v); };
      };

      auto non_predicate = negate_result(predicate);
      using NonPredicate = decltype(non_predicate);

      using result_true_range_type = ezy::detail::range_view_filter<
        ezy::experimental::reference_category_tag,
        const range_type,
        Predicate>;

      using result_false_range_type = ezy::detail::range_view_filter<
        ezy::experimental::reference_category_tag,
        const range_type,
        NonPredicate>;

      return std::make_tuple(
          ezy::make_strong<ezy::notag_t, has_iterator, algo_iterable>(
            result_true_range_type{ezy::experimental::reference_to<const range_type>(base::underlying()), predicate}
          ),
          ezy::make_strong<ezy::notag_t, has_iterator, algo_iterable>(
            result_false_range_type{ezy::experimental::reference_to<const range_type>(base::underlying()), non_predicate}
          )
          );
    }

    template <typename Predicate>
    auto partition(Predicate&& predicate) && = delete;

    auto slice(const unsigned from, const unsigned until) const &
    {
      return ezy::make_strong<ezy::notag_t, has_iterator, algo_iterable>(
          ezy::slice((*this).underlying(), from, until)
          );
    }

    auto slice(const unsigned from, const unsigned until) &&
    {
      return ezy::make_strong<ezy::notag_t, has_iterator, algo_iterable>(
          ezy::slice(std::move(*this).underlying(), from, until)
          );
    }

    template <typename Predicate>
    bool all(Predicate&& predicate) const
    {
      return std::all_of(base::underlying().begin(), base::underlying().end(), std::forward<Predicate>(predicate));
    }

    template <typename Predicate>
    bool any(Predicate&& predicate) const
    {
      return std::any_of(base::underlying().begin(), base::underlying().end(), std::forward<Predicate>(predicate));
    }

    template <typename Predicate>
    bool none(Predicate&& predicate) const
    {
      return std::none_of(base::underlying().begin(), base::underlying().end(), std::forward<Predicate>(predicate));
    }

    /*
    auto grouped(const unsigned group_size) const
    {
      using range_type = typename std::remove_reference<typename T::type>::type;
      using group_range_type = detail::range_view_slice<range_type>;
      using algo_iterable_group_type = strong_type<group_range_type, notag_t, has_iterator, algo_iterable>;
      using wrapper_range = detail::basic_range_view<algo_iterable_group_type>;
      using algo_iterable_wrapper_range = strong_type<wrapper_range, notag_t, has_iterator, algo_iterable>;
      //using algo_iterable_range_view
      return algo_iterable_wrapper_range(algo_iterable_group_type(base::underlying(), 0, group_size));
    }
    */

    template <typename... OtherRanges>
    auto zip(OtherRanges&&... other_ranges) const &
    {
      return ezy::make_strong<ezy::notag_t, has_iterator, algo_iterable>(
        ezy::zip((*this).underlying(), std::forward<OtherRanges>(other_ranges)...)
      );
    }

    template <typename... OtherRanges>
    auto zip(OtherRanges&&... other_ranges) &&
    {
      return ezy::make_strong<ezy::notag_t, has_iterator, algo_iterable>(
        ezy::zip(std::move(*this).underlying(), std::forward<OtherRanges>(other_ranges)...)
      );
    }

    template <typename Zipper, typename... OtherRanges>
    auto zip_with(Zipper&& zipper, OtherRanges&&... other_ranges) const &
    {
      return ezy::make_strong<ezy::notag_t, has_iterator, algo_iterable>(
        ezy::zip_with(
          std::forward<Zipper>(zipper),
          (*this).underlying(),
          std::forward<OtherRanges>(other_ranges)...)
      );
    }

    template <typename Zipper, typename... OtherRanges>
    auto zip_with(Zipper&& zipper, OtherRanges&&... other_ranges) &&
    {
      return ezy::make_strong<ezy::notag_t, has_iterator, algo_iterable>(
        ezy::zip_with(
          std::forward<Zipper>(zipper),
          std::move(*this).underlying(),
          std::forward<OtherRanges>(other_ranges)...)
      );
    }

    //template <typename OtherRange> // TODO constrain to be a raised (flattenable) range
    auto flatten() const &
    {
      return ezy::make_strong<ezy::notag_t, has_iterator, algo_iterable>(
        ezy::flatten((*this).underlying())
      );
    }

    auto flatten() &&
    {
      return ezy::make_strong<ezy::notag_t, has_iterator, algo_iterable>(
        ezy::flatten(std::move(*this).underlying())
      );
    }

    auto take(size_t n) const &
    {
      return ezy::make_strong<ezy::notag_t, has_iterator, algo_iterable>(
          ezy::take((*this).underlying(), n)
          );
    }

    auto take(size_t n) &&
    {
      return ezy::make_strong<ezy::notag_t, has_iterator, algo_iterable>(
        ezy::take(std::move(*this).underlying(), n)
      );
    }

    template <typename Predicate>
    auto take_while(Predicate&& pred) const &
    {
      return ezy::make_strong<ezy::notag_t, has_iterator, algo_iterable>(
          ezy::take_while((*this).underlying(), std::forward<Predicate>(pred))
          );
    }

    template <typename Predicate>
    auto take_while(Predicate&& pred) &&
    {
      return ezy::make_strong<ezy::notag_t, has_iterator, algo_iterable>(
          ezy::take_while(std::move(*this).underlying(), std::forward<Predicate>(pred))
          );
    }

    template <typename ResultContainer>
    constexpr ResultContainer to() const &
    {
      return ezy::collect<ResultContainer>((*this).underlying());
    }

    template <typename ResultContainer>
    constexpr ResultContainer to() &&
    {
      return ezy::collect<ResultContainer>(std::move(*this).underlying());
    }

    template <template <typename, typename...> class ResultWrapper>
    constexpr auto to() &&
    {
      return ezy::collect<ResultWrapper>(std::move(*this).underlying());
    }

    template <template <typename, typename...> class ResultWrapper>
    constexpr auto to() const &
    {
      return ezy::collect<ResultWrapper>((*this).underlying());
    }

    template <typename ResultContainer>
    constexpr auto to_iterable() const
    {
      return ezy::make_strong<ezy::notag_t, has_iterator, algo_iterable>(
          ezy::collect<ResultContainer>((*this).underlying())
          );
    }

    template <typename Separator>
    constexpr auto join(Separator&& separator) const
    {
      return ezy::join((*this).underlying(), std::forward<Separator>(separator));
    }

    constexpr auto enumerate() const &
    {
      return ezy::make_strong<ezy::notag_t, has_iterator, algo_iterable>(
        ezy::enumerate((*this).underlying())
        );
    }

    constexpr auto enumerate() &&
    {
      return ezy::make_strong<ezy::notag_t, has_iterator, algo_iterable>(
        ezy::enumerate(std::move(*this).underlying())
        );
    }

    constexpr auto cycle() const &
    {
      return ezy::make_strong<ezy::notag_t, has_iterator, algo_iterable>(
          ezy::cycle((*this).underlying())
          );
    }

    constexpr auto cycle() &&
    {
      return ezy::make_strong<ezy::notag_t, has_iterator, algo_iterable>(
          ezy::cycle(std::move(*this).underlying())
          );
    }
  };

  template <typename T>
  struct iterable : has_iterator<T>, algo_iterable<T> {};

}

#endif
