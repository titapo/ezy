#ifndef EZY_FEATURES_ALGO_ITERABLE_H_INCLUDED
#define EZY_FEATURES_ALGO_ITERABLE_H_INCLUDED

#include <ezy/feature.h>
#include <ezy/strong_type_traits.h>
#include <ezy/bits/algorithm.h>

namespace ezy
{
namespace features {

  namespace detail
  {
    template <typename T, typename Tag, typename WrappedFeatures>
    struct strong_type_from;

    template <typename T, typename Tag, template <typename> class... Features>
    struct strong_type_from<T, Tag, std::tuple<ezy::detail::feature_wrapper<Features>...>>
    {
      using type = ezy::strong_type<T, Tag, Features...>;
    };

    template <typename T, typename Tag, typename WrappedFeatures>
    using strong_type_from_t = typename strong_type_from<T, Tag, WrappedFeatures>::type;

    template <typename ReferenceST, typename T>
    constexpr auto make_extended_from(T&& t)
    {
      return strong_type_from_t<
        ezy::remove_cvref_t<T>,
        ezy::notag_t,
        ezy::extract_wrapped_features_t<ReferenceST>
      >(std::forward<T>(t));
    }
  }

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
      return detail::make_extended_from<T>(
          ezy::transform((*this).underlying(), std::forward<UnaryFunction>(f))
        );
    }

    template <typename UnaryFunction>
    constexpr auto map(UnaryFunction&& f) &&
    {
      return detail::make_extended_from<T>(
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
      return detail::make_extended_from<T>(
          ezy::concatenate((*this).underlying(), std::forward<RhsRange>(rhs))
          );
    }

    template <typename RhsRange>
    auto concatenate(RhsRange&& rhs) &&
    {
      return detail::make_extended_from<T>(
          ezy::concatenate(std::move(*this).underlying(), std::forward<RhsRange>(rhs))
          );
    }

    template <typename Predicate>
    auto filter(Predicate&& predicate) const &
    {
      return detail::make_extended_from<T>(
          ezy::filter((*this).underlying(), std::forward<Predicate>(predicate))
        );
    }

    template <typename Predicate>
    auto filter(Predicate&& predicate) &&
    {
      return detail::make_extended_from<T>(
          ezy::filter(std::move(*this).underlying(), std::forward<Predicate>(predicate))
        );
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
    Type accumulate(Type&& init) const
    {
      return ezy::accumulate((*this).underlying(), std::forward<Type>(init));
    }

    template <typename Type, typename BinaryOp>
    Type accumulate(Type&& init, BinaryOp&& op) const
    {
      return ezy::accumulate((*this).underlying(), std::forward<Type>(init), std::forward<BinaryOp>(op));
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

      return std::make_tuple(
          ezy::filter((*this).underlying(), predicate),
          ezy::filter((*this).underlying(), negate_result(predicate))
          );
    }

    template <typename Predicate>
    auto partition(Predicate&& predicate) && = delete;

    auto slice(const unsigned from, const unsigned until) const &
    {
      return detail::make_extended_from<T>(
          ezy::slice((*this).underlying(), from, until)
          );
    }

    auto slice(const unsigned from, const unsigned until) &&
    {
      return detail::make_extended_from<T>(
          ezy::slice(std::move(*this).underlying(), from, until)
          );
    }

    template <typename Predicate>
    bool all(Predicate&& predicate) const
    {
      return ezy::all_of(base::underlying(), std::forward<Predicate>(predicate));
    }

    template <typename Predicate>
    bool any(Predicate&& predicate) const
    {
      return ezy::any_of(base::underlying(), std::forward<Predicate>(predicate));
    }

    template <typename Predicate>
    bool none(Predicate&& predicate) const
    {
      return ezy::none_of(base::underlying(), std::forward<Predicate>(predicate));
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
      return detail::make_extended_from<T>(
        ezy::zip((*this).underlying(), std::forward<OtherRanges>(other_ranges)...)
      );
    }

    template <typename... OtherRanges>
    auto zip(OtherRanges&&... other_ranges) &&
    {
      return detail::make_extended_from<T>(
        ezy::zip(std::move(*this).underlying(), std::forward<OtherRanges>(other_ranges)...)
      );
    }

    template <typename Zipper, typename... OtherRanges>
    auto zip_with(Zipper&& zipper, OtherRanges&&... other_ranges) const &
    {
      return detail::make_extended_from<T>(
        ezy::zip_with(
          std::forward<Zipper>(zipper),
          (*this).underlying(),
          std::forward<OtherRanges>(other_ranges)...)
      );
    }

    template <typename Zipper, typename... OtherRanges>
    auto zip_with(Zipper&& zipper, OtherRanges&&... other_ranges) &&
    {
      return detail::make_extended_from<T>(
        ezy::zip_with(
          std::forward<Zipper>(zipper),
          std::move(*this).underlying(),
          std::forward<OtherRanges>(other_ranges)...)
      );
    }

    //template <typename OtherRange> // TODO constrain to be a raised (flattenable) range
    auto flatten() const &
    {
      return detail::make_extended_from<T>(
        ezy::flatten((*this).underlying())
      );
    }

    auto flatten() &&
    {
      return detail::make_extended_from<T>(
        ezy::flatten(std::move(*this).underlying())
      );
    }

    auto take(size_t n) const &
    {
      return detail::make_extended_from<T>(
          ezy::take((*this).underlying(), n)
          );
    }

    auto take(size_t n) &&
    {
      return detail::make_extended_from<T>(
        ezy::take(std::move(*this).underlying(), n)
      );
    }

    template <typename Predicate>
    auto take_while(Predicate&& pred) const &
    {
      return detail::make_extended_from<T>(
          ezy::take_while((*this).underlying(), std::forward<Predicate>(pred))
          );
    }

    template <typename Predicate>
    auto take_while(Predicate&& pred) &&
    {
      return detail::make_extended_from<T>(
          ezy::take_while(std::move(*this).underlying(), std::forward<Predicate>(pred))
          );
    }

    auto drop(size_t n) const &
    {
      return detail::make_extended_from<T>(
          ezy::drop((*this).underlying(), n)
          );
    }

    auto drop(size_t n) & /*mutable accessor*/
    {
      return detail::make_extended_from<T>(
          ezy::drop((*this).underlying(), n)
          );
    }

    auto drop(size_t n) &&
    {
      return detail::make_extended_from<T>(
          ezy::drop(std::move(*this).underlying(), n)
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
      return detail::make_extended_from<T>(
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
      return detail::make_extended_from<T>(
        ezy::enumerate((*this).underlying())
        );
    }

    constexpr auto enumerate() &&
    {
      return detail::make_extended_from<T>(
        ezy::enumerate(std::move(*this).underlying())
        );
    }

    constexpr auto cycle() const &
    {
      return detail::make_extended_from<T>(
          ezy::cycle((*this).underlying())
          );
    }

    constexpr auto cycle() &&
    {
      return detail::make_extended_from<T>(
          ezy::cycle(std::move(*this).underlying())
          );
    }
  };

}
}

#endif
