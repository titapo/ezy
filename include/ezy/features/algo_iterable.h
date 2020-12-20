#ifndef EZY_FEATURES_ALGO_ITERABLE_H_INCLUDED
#define EZY_FEATURES_ALGO_ITERABLE_H_INCLUDED

#include <ezy/strong_type_traits.h>
#include <ezy/bits/algorithm.h>

namespace ezy
{
namespace features {

  namespace detail
  {
    template <typename T, typename Tag, typename WrappedFeatures>
    struct strong_type_from;

    template <typename T, typename Tag, typename... Features>
    struct strong_type_from<T, Tag, std::tuple<Features...>>
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
        ezy::extract_features_t<ReferenceST>
      >(std::forward<T>(t));
    }
  }

  struct algo_iterable
  {
    template <typename T>
    struct impl
    {
      using _size_type = ezy::detail::size_type_t<ezy::extract_underlying_type_t<T>>;

      template < typename UnaryFunction >
      auto for_each(UnaryFunction f)
      {
        return ezy::for_each(static_cast<T&>(*this), std::forward<UnaryFunction>(f));
      }

      template <typename UnaryFunction>
      auto for_each(UnaryFunction f) const
      {
        return ezy::for_each(static_cast<const T&>(*this), std::forward<UnaryFunction>(f));
      }

      template <typename UnaryFunction>
      constexpr auto map(UnaryFunction&& f) const &
      {
        return detail::make_extended_from<T>(
            ezy::transform(static_cast<const T&>(*this).get(), std::forward<UnaryFunction>(f))
          );
      }

      template <typename UnaryFunction>
      constexpr auto map(UnaryFunction&& f) &&
      {
        return detail::make_extended_from<T>(
            ezy::transform(static_cast<T&&>(*this).get(), std::forward<UnaryFunction>(f))
          );
      }

      template <typename UnaryFunction>
      auto flat_map(UnaryFunction&& f) const &
      {
        return static_cast<const T&>(*this).flatten().map(std::forward<UnaryFunction>(f));
      }

      template <typename UnaryFunction>
      auto flat_map(UnaryFunction&& f) &&
      {
        return static_cast<T&&>(*this).flatten().map(std::forward<UnaryFunction>(f));
      }

      template <typename RhsRange>
      auto concatenate(RhsRange&& rhs) const &
      {
        return detail::make_extended_from<T>(
            ezy::concatenate(static_cast<const T&>(*this).get(), std::forward<RhsRange>(rhs))
            );
      }

      template <typename RhsRange>
      auto concatenate(RhsRange&& rhs) &&
      {
        return detail::make_extended_from<T>(
            ezy::concatenate(static_cast<T&&>(*this).get(), std::forward<RhsRange>(rhs))
            );
      }

      template <typename Predicate>
      auto filter(Predicate&& predicate) const &
      {
        return detail::make_extended_from<T>(
            ezy::filter(static_cast<const T&>(*this).get(), std::forward<Predicate>(predicate))
          );
      }

      template <typename Predicate>
      auto filter(Predicate&& predicate) &&
      {
        return detail::make_extended_from<T>(
            ezy::filter(static_cast<T&&>(*this).get(), std::forward<Predicate>(predicate))
          );
      }

      [[nodiscard]] constexpr auto empty() const
      {
        return ezy::empty(static_cast<const T&>(*this).get());
      }

      [[nodiscard]] constexpr auto size() const
      {
        return ezy::size(static_cast<const T&>(*this).get());
      }

      template <typename Element>
      bool contains(Element&& needle) const
      {
        return ezy::contains(static_cast<const T&>(*this).get(), std::forward<Element>(needle));
      }

      template <typename Type>
      Type accumulate(Type&& init) const
      {
        return ezy::accumulate(static_cast<const T&>(*this).get(), std::forward<Type>(init));
      }

      template <typename Type, typename BinaryOp>
      Type accumulate(Type&& init, BinaryOp&& op) const
      {
        return ezy::accumulate(static_cast<const T&>(*this).get(), std::forward<Type>(init), std::forward<BinaryOp>(op));
      }

      /*
       * not found in gcc even if numeric has been included
      template <typename Type>
      Type reduce(Type init)
      {
        return std::reduce(base::underlying().begin(), base::underlying().end(), init);
      }
      */

      auto chunk(_size_type chunk_size) const &
      {
        return detail::make_extended_from<T>(
            ezy::chunk(static_cast<const T&>(*this).get(), chunk_size)
            );
      }

      auto chunk(_size_type chunk_size) &
      {
        return detail::make_extended_from<T>(
            ezy::chunk(static_cast<T&>(*this).get(), chunk_size)
            );
      }

      auto chunk(_size_type chunk_size) &&
      {
        return detail::make_extended_from<T>(
            ezy::chunk(static_cast<T&&>(*this).get(), chunk_size)
            );
      }

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
            ezy::filter(static_cast<const T&>(*this).get(), predicate),
            ezy::filter(static_cast<const T&>(*this).get(), negate_result(predicate))
            );
      }

      template <typename Predicate>
      auto partition(Predicate&& predicate) && = delete;

      auto slice(const unsigned from, const unsigned until) const &
      {
        return detail::make_extended_from<T>(
            ezy::slice(static_cast<const T&>(*this).get(), from, until)
            );
      }

      auto slice(const unsigned from, const unsigned until) &&
      {
        return detail::make_extended_from<T>(
            ezy::slice(static_cast<T&&>(*this).get(), from, until)
            );
      }

      template <typename Predicate>
      bool all(Predicate&& predicate) const
      {
        return ezy::all_of(static_cast<const T&>(*this), std::forward<Predicate>(predicate));
      }

      template <typename Predicate>
      bool any(Predicate&& predicate) const
      {
        return ezy::any_of(static_cast<const T&>(*this), std::forward<Predicate>(predicate));
      }

      template <typename Predicate>
      bool none(Predicate&& predicate) const
      {
        return ezy::none_of(static_cast<const T&>(*this), std::forward<Predicate>(predicate));
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
          ezy::zip(static_cast<const T&>(*this).get(), std::forward<OtherRanges>(other_ranges)...)
        );
      }

      template <typename... OtherRanges>
      auto zip(OtherRanges&&... other_ranges) &&
      {
        return detail::make_extended_from<T>(
          ezy::zip(static_cast<T&&>(*this).get(), std::forward<OtherRanges>(other_ranges)...)
        );
      }

      template <typename Zipper, typename... OtherRanges>
      auto zip_with(Zipper&& zipper, OtherRanges&&... other_ranges) const &
      {
        return detail::make_extended_from<T>(
          ezy::zip_with(
            std::forward<Zipper>(zipper),
            static_cast<const T&>(*this).get(),
            std::forward<OtherRanges>(other_ranges)...)
        );
      }

      template <typename Zipper, typename... OtherRanges>
      auto zip_with(Zipper&& zipper, OtherRanges&&... other_ranges) &&
      {
        return detail::make_extended_from<T>(
          ezy::zip_with(
            std::forward<Zipper>(zipper),
            static_cast<T&&>(*this).get(),
            std::forward<OtherRanges>(other_ranges)...)
        );
      }

      //template <typename OtherRange> // TODO constrain to be a raised (flattenable) range
      auto flatten() const &
      {
        return detail::make_extended_from<T>(
          ezy::flatten(static_cast<const T&>(*this).get())
        );
      }

      auto flatten() &&
      {
        return detail::make_extended_from<T>(
          ezy::flatten(static_cast<T&&>(*this).get())
        );
      }

      auto take(size_t n) const &
      {
        return detail::make_extended_from<T>(
            ezy::take(static_cast<const T&>(*this).get(), n)
            );
      }

      auto take(size_t n) &&
      {
        return detail::make_extended_from<T>(
          ezy::take(static_cast<T&&>(*this).get(), n)
        );
      }

      template <typename Predicate>
      auto take_while(Predicate&& pred) const &
      {
        return detail::make_extended_from<T>(
            ezy::take_while(static_cast<const T&>(*this).get(), std::forward<Predicate>(pred))
            );
      }

      template <typename Predicate>
      auto take_while(Predicate&& pred) &&
      {
        return detail::make_extended_from<T>(
            ezy::take_while(static_cast<T&&>(*this).get(), std::forward<Predicate>(pred))
            );
      }

      auto drop(size_t n) const &
      {
        return detail::make_extended_from<T>(
            ezy::drop(static_cast<const T&>(*this).get(), n)
            );
      }

      auto drop(size_t n) & /*mutable accessor*/
      {
        return detail::make_extended_from<T>(
            ezy::drop(static_cast<T&>(*this).get(), n)
            );
      }

      auto drop(size_t n) &&
      {
        return detail::make_extended_from<T>(
            ezy::drop(static_cast<T&&>(*this).get(), n)
            );
      }

      template <typename ResultContainer>
      constexpr ResultContainer to() const &
      {
        return ezy::collect<ResultContainer>(static_cast<const T&>(*this).get());
      }

      template <typename ResultContainer>
      constexpr ResultContainer to() &&
      {
        return ezy::collect<ResultContainer>(static_cast<T&&>(*this).get());
      }

      template <template <typename, typename...> class ResultWrapper>
      constexpr auto to() &&
      {
        return ezy::collect<ResultWrapper>(static_cast<T&&>(*this).get());
      }

      template <template <typename, typename...> class ResultWrapper>
      constexpr auto to() const &
      {
        return ezy::collect<ResultWrapper>(static_cast<const T&>(*this).get());
      }

      template <typename ResultContainer>
      constexpr auto to_iterable() const
      {
        return detail::make_extended_from<T>(
            ezy::collect<ResultContainer>(static_cast<const T&>(*this).get())
            );
      }

      template <typename Separator>
      constexpr auto join(Separator&& separator) const
      {
        return ezy::join(static_cast<const T&>(*this).get(), std::forward<Separator>(separator));
      }

      constexpr auto enumerate() const &
      {
        return detail::make_extended_from<T>(
          ezy::enumerate(static_cast<const T&>(*this).get())
          );
      }

      constexpr auto enumerate() &&
      {
        return detail::make_extended_from<T>(
          ezy::enumerate(static_cast<T&&>(*this).get())
          );
      }

      constexpr auto cycle() const &
      {
        return detail::make_extended_from<T>(
            ezy::cycle(static_cast<const T&>(*this).get())
            );
      }

      constexpr auto cycle() &&
      {
        return detail::make_extended_from<T>(
            ezy::cycle(static_cast<T&&>(*this).get())
            );
      }
    };
  };

}
}

#endif
