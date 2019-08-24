#ifndef EZY_FEATURES_ITERABLE_H_INCLUDED
#define EZY_FEATURES_ITERABLE_H_INCLUDED

#include <algorithm>
#include <functional>
#include <numeric> // for accumulate and reduce

#include "../range.h"
#include "../utility.h" // for ezy::optional

#include <experimental/type_traits>

template <typename Range, typename Key>
using find_mem_fn_t = decltype(std::declval<Range>().find(std::declval<Key>()));

template <typename Range>
using size_mem_fn_t = decltype(std::declval<Range>().size());

template <typename Range>
using empty_mem_fn_t = decltype(std::declval<Range>().empty());

namespace ezy::features
{
  template <typename T>
  struct has_iterator : crtp<T, has_iterator>
  {
    auto begin() { return std::begin(this->that().get()); }
    auto begin() const { return std::begin(this->that().get()); }
    auto cbegin() { return std::cbegin(this->that().get()); }
    auto cbegin() const { return std::cbegin(this->that().get()); }

    auto end() { return std::end(this->that().get()); }
    auto end() const { return std::end(this->that().get()); }
    auto cend() { return std::cend(this->that().get()); }
    auto cend() const { return std::cend(this->that().get()); }
  };

  template <typename T>
  struct inherit_range_view_features : crtp<T, inherit_range_view_features>
  {
    using const_iterator = typename T::const_iterator;
  };

  template <typename T>
  struct algo_iterable : crtp<T, algo_iterable>
  {
    template < typename UnaryFunction >
    auto for_each(UnaryFunction f)
    {
      return std::for_each(this->that().get().begin(), this->that().get().end(), f);
    }

    template <typename UnaryFunction>
    auto for_each(UnaryFunction f) const
    {
      return std::for_each(this->that().get().begin(), this->that().get().end(), f);
    }

    template <typename UnaryFunction>
    auto map(UnaryFunction&& f) const
    {
      using range_type = typename std::remove_reference<typename T::type>::type;
      using result_range_type = range_view<range_type, UnaryFunction>;
      using algo_iterable_range_view = strong_type<result_range_type, notag_t, has_iterator, algo_iterable>;
      return algo_iterable_range_view(result_range_type(this->that().get(), std::forward<UnaryFunction>(f)));
    }

    template <typename UnaryFunction>
    auto flat_map(UnaryFunction&& f) const
    {
      // FIXME
      using range_type = typename std::remove_reference<typename T::type>::type;
      using result_range_type = range_view<range_type, UnaryFunction>;
      using algo_iterable_range_view = strong_type<result_range_type, notag_t, has_iterator, algo_iterable>;
      return algo_iterable_range_view(result_range_type(this->that().get(), std::forward<UnaryFunction>(f)));
    }

    template <typename RhsRange>
    auto concatenate(const RhsRange& rhs) const
    {
      using range_type = typename std::remove_reference<typename T::type>::type;
      using result_range_type = concatenated_range_view<range_type, RhsRange>;
      using algo_iterable_range_view = strong_type<result_range_type, notag_t, has_iterator, algo_iterable>;
      return algo_iterable_range_view(result_range_type(this->that().get(), rhs));
    }

    template <typename Predicate>
    auto filter(Predicate&& predicate) const
    {
      using range_type = typename std::remove_reference<typename T::type>::type;
      using result_range_type = range_view_filter<range_type, Predicate>;
      using algo_iterable_range_view = strong_type<result_range_type, notag_t, has_iterator, algo_iterable>;
      return algo_iterable_range_view(result_range_type(this->that().get(), std::forward<Predicate>(predicate)));
    }

    template <typename Element>
    auto find(Element&& element) const
    {
      using range_type = typename std::remove_reference<typename T::type>::type;
      using result_type = ezy::optional<typename range_type::value_type>;

      if constexpr (std::experimental::is_detected<find_mem_fn_t, typename T::type, Element>::value)
      {
        const auto found = this->that().get().find(std::forward<Element>(element));
        if (found != this->that().get().end())
          return result_type(*found);
        else
          return result_type();
      }
      else
      {
        const auto found = std::find(this->that().get().begin(), this->that().get().end(), std::forward<Element>(element));
        if (found != this->that().get().end())
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
      const auto found = std::find_if(this->that().get().begin(), this->that().get().end(), std::forward<Predicate>(predicate));
      if (found != this->that().get().end())
        return result_type(*found);
      else
        return result_type();
    }

    auto empty() const
    {
      if constexpr (std::experimental::is_detected<empty_mem_fn_t, typename T::type>::value)
      {
        return this->that().get().empty();
      }
      else
      {
        return !(this->that().get().begin() != this->that().get().end());
      }
    }

    auto size() const
    {
      if constexpr (std::experimental::is_detected<size_mem_fn_t, typename T::type>::value)
      {
        return this->that().get().size();
      }
      else
      {
        return std::distance(this->that().get().begin(), this->that().get().end());
      }
    }

    template <typename Element>
    bool contains(Element&& needle) const
    {
      return find(std::forward<Element>(needle)).has_value();
    }

    template <typename Type>
    Type accumulate(Type init) 
    {
      return std::accumulate(this->that().get().begin(), this->that().get().end(), init);
    }

    template <typename Type, typename BinaryOp>
    Type accumulate(Type init, BinaryOp op) 
    {
      return std::accumulate(this->that().get().begin(), this->that().get().end(), init, op);
    }

    /*
     * not found in gcc even if numeric has been included
    template <typename Type>
    Type reduce(Type init) 
    {
      return std::reduce(this->that().get().begin(), this->that().get().end(), init);
    }
    */

    template <typename Predicate>
    auto partition(Predicate&& predicate) const
    {
      using range_type = typename std::remove_reference<typename T::type>::type;
      using concrete_predicate_type = std::function<bool(const typename range_type::value_type&)>;
      using result_range_type = range_view_filter<range_type, concrete_predicate_type>;
      using algo_iterable_range_view = strong_type<result_range_type, notag_t, has_iterator, algo_iterable>;

      const auto negate_result = [](const Predicate& original_predicate)
      {
        return [=](const typename range_type::value_type& v)
        { return !original_predicate(v); };
      };

      return std::make_tuple(
          algo_iterable_range_view(result_range_type(this->that().get(), predicate)),
          algo_iterable_range_view(result_range_type(this->that().get(), negate_result(predicate)))
          );
    }

    auto slice(const unsigned from, const unsigned until) const
    {
      using range_type = typename std::remove_reference<typename T::type>::type;
      using result_range_type = range_view_slice<range_type>;
      using algo_iterable_range_view = strong_type<result_range_type, notag_t, has_iterator, algo_iterable>;
      return algo_iterable_range_view(result_range_type(this->that().get(), from, until));
    }

    template <typename Predicate>
    bool all(Predicate&& predicate) const
    {
      return std::all_of(this->that().get().begin(), this->that().get().end(), std::forward<Predicate>(predicate));
    }

    template <typename Predicate>
    bool any(Predicate&& predicate) const
    {
      return std::any_of(this->that().get().begin(), this->that().get().end(), std::forward<Predicate>(predicate));
    }

    template <typename Predicate>
    bool none(Predicate&& predicate) const
    {
      return std::none_of(this->that().get().begin(), this->that().get().end(), std::forward<Predicate>(predicate));
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
      return algo_iterable_wrapper_range(algo_iterable_group_type(this->that().get(), 0, group_size));
    }
    */

    template <typename OtherRange>
    auto zip(const OtherRange& other_range)
    {
      using lhs_range_type = typename std::remove_reference<typename T::type>::type;
      using rhs_range_type = typename std::remove_reference<OtherRange>::type;
      using result_range_type = zip_range_view<lhs_range_type, rhs_range_type>;
      using algo_iterable_range_view = strong_type<result_range_type, notag_t, has_iterator, algo_iterable>;
      return algo_iterable_range_view(result_range_type(this->that().get(), other_range));
    }

    //template <typename OtherRange> // TODO constrain to be a raised (flattenable) range
    auto flatten() const
    {
      using ThisRange = typename std::remove_reference<typename T::type>::type;
      using result_range_type = flattened_range_view<ThisRange> ;
      using algo_iterable_range_view = strong_type<result_range_type, notag_t, has_iterator, algo_iterable>;
      return algo_iterable_range_view(result_range_type(this->that().get()));
    }

    template <typename ResultContainer>
    ResultContainer to() const
    {
      return ResultContainer(this->that().get().begin(), this->that().get().end());
    }

    template <typename ResultContainer>
    auto to_iterable() const
    {
      using algo_iterable_container = strong_type<ResultContainer, notag_t, has_iterator, algo_iterable>;
      return algo_iterable_container(this->that().get().begin(), this->that().get().end());
    }

  };

  template <typename T>
  struct iterable : has_iterator<T>, algo_iterable<T> {};

}

#endif
