#ifndef EZY_BITS_FIND_H_INCLUDED
#define EZY_BITS_FIND_H_INCLUDED

#include "algorithm.h" // find_element*

#include <ezy/optional>
#include <ezy/pointer.h>

namespace ezy
{
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

    template <typename Range, typename Iterator>
    auto make_find_result(Iterator it, Iterator last)
    {
      using ValueType = std::remove_reference_t<decltype(*begin(std::declval<Range>()))>;
      using ReturnType = ezy::conditional_t<
        std::is_lvalue_reference_v<Range>,
        const ezy::pointer<ValueType>,
        ezy::optional<ValueType>
      >;

    if (it != last)
      return ReturnType(detail::dependent_forward<Range>(*it));
    else
      return ReturnType();
    }
  }

  struct find_fn
  {
    template <typename Range, typename Needle>
    /*constexpr*/ auto operator()(Range&& range, Needle&& needle) const
    {
      auto found = find_element(range, std::forward<Needle>(needle));
      return detail::make_find_result<Range>(found, end(range));
    }
  };

  static constexpr find_fn find{};

  struct find_if_fn
  {
    template <typename Range, typename Predicate>
    /*constexpr*/ auto operator()(Range&& range, Predicate&& pred) const
    {
      const auto found = find_element_if(range, std::forward<Predicate>(pred));
      return detail::make_find_result<Range>(found, end(range));
    }
  };

  static constexpr find_if_fn find_if{};
}

#endif
