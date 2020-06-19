#ifndef EZY_INVOKE_H_INCLUDED
#define EZY_INVOKE_H_INCLUDED

#include <type_traits>
#include <utility>

namespace ezy
{
  template <typename T, typename FnRet, typename Obj, typename...Args>
  constexpr decltype(auto) invoke(FnRet T::* fn, Obj&& t, Args&&... args)
  {
    using Fn = decltype(fn);
    if constexpr (std::is_member_function_pointer_v<Fn>)
    {
      return ((t).*fn)(std::forward<Args>(args)...);
    }
    else
    {
      static_assert(sizeof...(Args) == 0, "Args cannot be provided for member pointer");
      return t.*fn;
    }
  }

  template <typename Fn, typename... Args>
  constexpr decltype(auto) invoke(Fn&& fn, Args&&... args) noexcept(std::is_nothrow_invocable_v<Fn, Args...>)
  {
    return std::forward<Fn>(fn)(std::forward<Args>(args)...);
  }
}

#endif
