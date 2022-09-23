#ifndef EZY_BITS_DEPENDENT_FORWARD_H_INCLUDED
#define EZY_BITS_DEPENDENT_FORWARD_H_INCLUDED

#include <ezy/experimental/keeper.h>

namespace ezy
{
  namespace detail
  {
    template <typename T>
    decltype(auto) dependent_forward_impl(T&& t, ezy::experimental::owner_category_tag, std::false_type)
    { return std::move(t); }

    template <typename T>
    decltype(auto) dependent_forward_impl(T&& t, ezy::experimental::reference_category_tag, std::false_type)
    { return t; }

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
}

#endif
