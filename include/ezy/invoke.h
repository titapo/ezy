#ifndef EZY_INVOKE_H_INCLUDED
#define EZY_INVOKE_H_INCLUDED

#include <ezy/typelist_traits.h> // only for typelist
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
  constexpr auto invoke(Fn&& fn, Args&&... args)
      noexcept(noexcept(std::forward<Fn>(fn)(std::forward<Args>(args)...)))
      -> decltype(std::forward<Fn>(fn)(std::forward<Args>(args)...))
  {
    return std::forward<Fn>(fn)(std::forward<Args>(args)...);
  }

  namespace detail
  {

    template <typename Fn, typename... Args>
    struct is_invocable_helper
    {
      private:
      public:
        struct invalid_type {};

        template <typename SFn, typename... SArgs>
        static auto try_invoke(int) -> decltype(ezy::invoke(std::declval<SFn>(), std::declval<SArgs>()...));

        template <typename SFn, typename... SArgs>
        static invalid_type try_invoke(...);

        using Result = decltype(try_invoke<Fn, Args...>(0));

      public:
        static constexpr bool value = !std::is_same_v<Result, invalid_type>;
    };
  }


  template <typename Fn, typename... Args>
  struct is_invocable : std::integral_constant<bool, detail::is_invocable_helper<Fn, Args...>::value>
  {};

  template <typename Fn, typename... Args>
  static constexpr bool is_invocable_v = is_invocable<Fn, Args...>::value;

}

#endif
