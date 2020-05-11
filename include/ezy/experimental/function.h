#ifndef EZY_EXPERIMENTAL_FUNCTION_HH
#define EZY_EXPERIMENTAL_FUNCTION_HH

#include <functional>
#include <ezy/invoke.h>

namespace ezy::experimental::function
{

  /**
   * curry
   */
  template <typename T, typename U, typename F, typename = std::is_invocable<F(T, T)>>
  decltype(auto) curry(F&& f)
  {
    // TODO accept different references
    return [=](T a) {
      return [=](U b) {
        return std::invoke(f, a, b);
      };
    };
  }

  /**
   * compose
   */
  template <typename... Fs>
  struct composed
  {
    template <typename... Functions> //convertible to?
    constexpr explicit composed(Functions&&... fns)
      : fs{std::forward<Functions>(fns)...}
    {}

    static_assert(sizeof...(Fs) > 0, "Composing nothing is not supported.");
    std::tuple<Fs...> fs;

    template <typename...Fns, typename Fn, typename T>
    constexpr static decltype(auto) call_helper_pack(T&& t, Fn&& fn, Fns&&... fns)
    {
      if constexpr (sizeof...(Fns) == 0)
      {
        return ezy::invoke(std::forward<Fn>(fn), std::forward<T>(t));
      }
      else
      {
        return call_helper_pack(ezy::invoke(std::forward<Fn>(fn), std::forward<T>(t)), std::forward<Fns>(fns)...);
      }
    }

    template <typename T, typename Tuple, std::size_t...Is>
    constexpr static decltype(auto) call_helper_tuple(T&& t, Tuple&& tup, std::index_sequence<Is...>)
    {
      return call_helper_pack(std::forward<T>(t), std::get<Is>(tup)...);
    }

    template <typename...Fns, typename T>
    constexpr static decltype(auto) call_helper(std::tuple<Fns...> tup, T&& t)
    {
      return call_helper_tuple(
          std::forward<T>(t),
          std::forward<decltype(tup)>(tup),
          std::make_index_sequence<sizeof...(Fns)>());
    }

    template <typename T>
    constexpr decltype(auto) operator()(T&& t) const
    {
      return call_helper(fs, std::forward<T>(t));
    }
  };

  //template <typename F1, typename F2> composed(F1, F2) -> composed<F1, F2>;

  template <typename... Fns>
  constexpr composed<Fns...> compose(Fns&&... fns)
  {
    return composed<Fns...>(std::forward<Fns>(fns)...);
  }

}

#endif
