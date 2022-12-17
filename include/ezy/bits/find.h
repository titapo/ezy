#ifndef EZY_BITS_FIND_H_INCLUDED
#define EZY_BITS_FIND_H_INCLUDED

#include <ezy/algorithm/find_element.h>

#include <ezy/optional.h>
#include <ezy/pointer.h>

#include <ezy/bits/optional_like_value.h>

namespace ezy
{
  namespace detail
  {
    template <typename Range, typename Iterator>
    auto make_find_result(Iterator it, Iterator last)
    {
      if (it != last)
      {
        return make_optional_like_value<Range>(*it);
      }
      else
      {
        return make_optional_like_value<Range>();
      }
    }
  }

  struct find_fn
  {
    template <typename Range, typename Needle>
    /*constexpr*/ auto operator()(Range&& range, Needle&& needle) const
    {
      const auto found = find_element(range, std::forward<Needle>(needle));
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
