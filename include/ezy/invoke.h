#ifndef EZY_INVOKE_H_INCLUDED
#define EZY_INVOKE_H_INCLUDED

#include <type_traits>
#include <utility>

namespace ezy
{
  namespace detail
  {
    // member function pointer
    template <typename T, typename FnRet, typename Obj, typename...Args>
    constexpr decltype(auto) invoke_member(std::true_type, FnRet T::* fn, Obj&& t, Args&&... args)
    {
      return ((t).*fn)(std::forward<Args>(args)...);
    }

    // member pointer
    template <typename T, typename FnRet, typename Obj, typename...Args>
    constexpr decltype(auto) invoke_member(std::false_type, FnRet T::* fn, Obj&& t, Args&&... args)
    {
      static_assert(sizeof...(Args) == 0, "Args cannot be provided for member pointer");
      return t.*fn;
    }
  }

  template <typename T, typename FnRet, typename Obj, typename...Args>
  constexpr decltype(auto) invoke(FnRet T::* fn, Obj&& t, Args&&... args)
  {
    using Fn = decltype(fn);
    return detail::invoke_member(
        std::is_member_function_pointer<Fn>{},
        fn,
        std::forward<Obj>(t),
        std::forward<Args>(args)...);
  }

  template <typename Fn, typename... Args>
  constexpr decltype(auto) invoke(Fn&& fn, Args&&... args)
      noexcept(noexcept(std::forward<Fn>(fn)(std::forward<Args>(args)...)))
  {
    return std::forward<Fn>(fn)(std::forward<Args>(args)...);
  }
}

#endif
