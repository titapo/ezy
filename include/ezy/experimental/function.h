#ifndef EZY_EXPERIMENTAL_FUNCTION_HH
#define EZY_EXPERIMENTAL_FUNCTION_HH

#include <functional>
#include <ezy/invoke.h>

namespace ezy
{
namespace experimental
{
  namespace detail
  {
    struct piecewise_tag_t {};
  }

  template <typename Fn, typename... Args>
  struct curried_with_args
  {

    curried_with_args() = default;

    // enabed only if Args is empty
    constexpr curried_with_args(Fn fn)
      : storage{std::forward<Fn>(fn)}
    {}

    // for internal use only
    template <typename... Ts>
    constexpr explicit curried_with_args(detail::piecewise_tag_t, Ts&&... ts)
      : storage{std::forward<Ts>(ts)...}
    {}

    // this case should be handled differently
    constexpr decltype(auto) operator()() const noexcept
    {
      return std::get<0>(storage);
    }

    template <typename U>
    constexpr auto operator()(U&& u) const noexcept
    {
      return impl_call(std::forward<U>(u), std::is_invocable<Fn, Args..., U>{});
    }

    private:
      using Tuple = std::tuple<Fn, Args...>;
      using IndexSequence = std::index_sequence_for<Fn, Args...>;
      std::tuple<Fn, Args...> storage;

      // invocable
      template <typename U>
      constexpr auto impl_call(U&& u, std::true_type) const
      {
        return apply(storage, std::forward<U>(u), IndexSequence{});
      }

      // not invocable
      template <typename U>
      constexpr auto impl_call(U&& u, std::false_type) const
      {
        return defer(storage, std::forward<U>(u), IndexSequence{});
      }

      template <typename U, size_t... Is>
      static constexpr decltype(auto) apply(const Tuple& tup, U&& u, std::index_sequence<Is...>)
      {
        return ezy::invoke(std::get<Is>(tup)..., std::forward<U>(u));
      }

      template <typename U, size_t... Is>
      static constexpr decltype(auto) defer(const Tuple& tup, U&& u, std::index_sequence<Is...>)
      {
        return curried_with_args<Fn, Args..., U>(detail::piecewise_tag_t{}, std::get<Is>(tup)..., std::forward<U>(u));
      }
  };

  template <typename T> curried_with_args(T) -> curried_with_args<T>;

  template <typename T>
  struct curried : curried_with_args<T> {};

  template <typename T> curried(T) -> curried<T>;

  template <typename T>
  constexpr curried<T> curry(T&& t) noexcept
  {
    return {std::forward<T>(t)};
  }


  template <typename Fn>
  constexpr auto flip(Fn&& fn) noexcept
  {
    return [fn](auto&& b, auto&& a, auto&&... rest)
    {
      return ezy::invoke(fn,
          std::forward<decltype(a)>(a),
          std::forward<decltype(b)>(b),
          std::forward<decltype(rest)>(rest)...
          );
    };
  }

  /**
   * curry
   */
  template <typename T, typename U, typename F, typename = std::is_invocable<F(T, T)>>
  decltype(auto) curry_as(F&& f)
  {
    return [=](T a) {
      return [=](U b) {
        return ezy::invoke(f, a, b);
      };
    };
  }

}}

#endif
