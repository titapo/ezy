#ifndef EZY_FEATURES_ITERABLE_H_INCLUDED
#define EZY_FEATURES_ITERABLE_H_INCLUDED

#include <algorithm>
#include <functional>
#include <numeric> // for accumulate and reduce

#include "../range.h"
#include "../optional" // for ezy::optional
#include "../algorithm.h"

#include <experimental/type_traits>

template <typename Range, typename Key>
using find_mem_fn_t = decltype(std::declval<Range>().find(std::declval<Key>()));

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
    auto concatenate(const RhsRange& rhs) const &
    {
      using range_type = typename std::remove_reference<typename T::type>::type;
      using result_range_type = concatenated_range_view<ezy::experimental::reference_category_tag, const range_type, RhsRange>;
      using algo_iterable_range_view = strong_type<result_range_type, notag_t, has_iterator, algo_iterable>;
      return algo_iterable_range_view(result_range_type(ezy::experimental::reference_to<const range_type>(base::underlying()), rhs));
    }

    template <typename RhsRange>
    auto concatenate(const RhsRange& rhs) &&
    {
      using range_type = typename std::remove_reference<typename T::type>::type;
      using result_range_type = concatenated_range_view<ezy::experimental::owner_category_tag, range_type, RhsRange>;
      using algo_iterable_range_view = strong_type<result_range_type, notag_t, has_iterator, algo_iterable>;
      return algo_iterable_range_view(result_range_type(ezy::experimental::owner<range_type>(std::move(*this).underlying()), rhs));
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
      using range_type = typename std::remove_reference<typename T::type>::type;
      using result_type = ezy::optional<typename range_type::value_type>;

      if constexpr (std::experimental::is_detected<find_mem_fn_t, typename T::type, Element>::value)
      {
        const auto found = base::underlying().find(std::forward<Element>(element));
        if (found != base::underlying().end())
          return result_type(*found);
        else
          return result_type();
      }
      else
      {
        const auto found = std::find(base::underlying().begin(), base::underlying().end(), std::forward<Element>(element));
        if (found != base::underlying().end())
          return result_type(*found);
        else
          return result_type();
      }
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
      return find(std::forward<Element>(needle)).has_value();
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
      using concrete_predicate_type = std::function<bool(const typename range_type::value_type&)>; // TODO
      using result_range_type = range_view_filter<ezy::experimental::reference_category_tag, const range_type, concrete_predicate_type>;
      using algo_iterable_range_view = strong_type<result_range_type, notag_t, has_iterator, algo_iterable>;

      const auto negate_result = [](const Predicate& original_predicate)
      {
        return [=](const typename range_type::value_type& v)
        { return !original_predicate(v); };
      };

      return std::make_tuple(
          algo_iterable_range_view(result_range_type{ezy::experimental::reference_to<const range_type>(base::underlying()), predicate}),
          algo_iterable_range_view(result_range_type{ezy::experimental::reference_to<const range_type>(base::underlying()), negate_result(predicate)})
          );
    }

    template <typename Predicate>
    auto partition(Predicate&& predicate) && = delete;

    auto slice(const unsigned from, const unsigned until) const &
    {
      using range_type = typename std::remove_reference<typename T::type>::type;
      using result_range_type = range_view_slice<ezy::experimental::reference_category_tag, const range_type>;
      using algo_iterable_range_view = strong_type<result_range_type, notag_t, has_iterator, algo_iterable>;
      return algo_iterable_range_view(result_range_type{ezy::experimental::reference_to<const range_type>(base::underlying()), from, until});
    }

    auto slice(const unsigned from, const unsigned until) &&
    {
      using range_type = typename std::remove_reference<typename T::type>::type;
      using result_range_type = range_view_slice<ezy::experimental::owner_category_tag, range_type>;
      using algo_iterable_range_view = strong_type<result_range_type, notag_t, has_iterator, algo_iterable>;
      return algo_iterable_range_view(result_range_type{ezy::experimental::owner<range_type>(std::move(*this).underlying()), from, until});
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
      using group_range_type = range_view_slice<range_type>;
      using algo_iterable_group_type = strong_type<group_range_type, notag_t, has_iterator, algo_iterable>;
      using wrapper_range = basic_range_view<algo_iterable_group_type>;
      using algo_iterable_wrapper_range = strong_type<wrapper_range, notag_t, has_iterator, algo_iterable>;
      //using algo_iterable_range_view
      return algo_iterable_wrapper_range(algo_iterable_group_type(base::underlying(), 0, group_size));
    }
    */

    template <typename OtherRange>
    auto zip(const OtherRange& other_range)
    {
      using lhs_range_type = typename std::remove_reference<typename T::type>::type;
      using rhs_range_type = typename std::remove_reference<OtherRange>::type;
      using result_range_type = zip_range_view<lhs_range_type, rhs_range_type>;
      using algo_iterable_range_view = strong_type<result_range_type, notag_t, has_iterator, algo_iterable>;
      return algo_iterable_range_view(result_range_type(base::underlying(), other_range));
    }

    //template <typename OtherRange> // TODO constrain to be a raised (flattenable) range
    auto flatten() const &
    {
      using ThisRange = typename std::remove_reference<typename T::type>::type;
      using result_range_type = flattened_range_view<ezy::experimental::reference_category_tag, const ThisRange>;
      using algo_iterable_range_view = strong_type<result_range_type, notag_t, has_iterator, algo_iterable>;
      return algo_iterable_range_view(result_range_type{ezy::experimental::reference_to<const ThisRange>(base::underlying())});
    }

    auto flatten() &&
    {
      using ThisRange = typename std::remove_reference<typename T::type>::type;
      using result_range_type = flattened_range_view<ezy::experimental::owner_category_tag, ThisRange>;
      using algo_iterable_range_view = strong_type<result_range_type, notag_t, has_iterator, algo_iterable>;
      return algo_iterable_range_view(result_range_type{ezy::experimental::owner<ThisRange>(std::move(*this).underlying())});
    }

    auto take(size_t n) const
    {
      using this_range = std::remove_reference_t<typename T::type>;
      using result_range = take_n_range_view<this_range>;
      using algo_iterable_range_view = strong_type<result_range, notag_t, has_iterator, algo_iterable>;
      return algo_iterable_range_view(result_range(base::underlying(), n));
    }

    template <typename Predicate>
    auto take_while(Predicate&& pred) const
    {
      using this_range = std::remove_reference_t<typename T::type>;
      using result_range = take_while_range_view<this_range, ezy::remove_cvref_t<Predicate>>;
      using algo_iterable_range_view = strong_type<result_range, notag_t, has_iterator, algo_iterable>;
      return algo_iterable_range_view(result_range(base::underlying(), std::forward<Predicate>(pred)));
    }

    template <typename ResultContainer>
    constexpr ResultContainer to() const
    {
      return ResultContainer(base::underlying().begin(), base::underlying().end());
    }

    template <typename ResultContainer>
    constexpr auto to_iterable() const
    {
      using algo_iterable_container = strong_type<ResultContainer, notag_t, has_iterator, algo_iterable>;
      return algo_iterable_container(base::underlying().begin(), base::underlying().end());
    }

  };

  template <typename T>
  struct iterable : has_iterator<T>, algo_iterable<T> {};

}

#endif
