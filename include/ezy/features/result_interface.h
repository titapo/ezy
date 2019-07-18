#ifndef EZY_FEATURES_RESULT_INTERFACE_INCLUDED
#define EZY_FEATURES_RESULT_INTERFACE_INCLUDED

#include "../strong_type.h"

#include <functional>
#include <variant> // std::get

namespace ezy::features
{
  template <typename Type>
  struct result_adapter;

  template <template <typename...> class Wrapper, typename Success, typename Error>
  struct result_adapter<Wrapper<Success, Error>>
  {
    using type = Wrapper<Success, Error>;
    inline static constexpr size_t success = 0;
    inline static constexpr size_t error = 1;
    using success_type = std::decay_t<decltype(std::get<success>(std::declval<type>()))>;
    using error_type = std::decay_t<decltype(std::get<error>(std::declval<type>()))>;

    template <typename T>
    static bool is_success(T&& t) noexcept
    {
      return t.index() == success;
    }

    template <typename T>
    static decltype(auto) get_success(T&& t) noexcept
    {
      return std::get<success>(std::forward<T>(t));
    }

    template <typename T>
    static decltype(auto) get_error(T&& t) noexcept
    {
      return std::get<error>(std::forward<T>(t));
    }

    template <typename... Ts>
    static decltype(auto) make_success(Ts&&... ts)
    {
      return type{std::in_place_index_t<success>{}, std::forward<Ts>(ts)...};
    }

    template <typename... Ts>
    static decltype(auto) make_error(Ts&&... ts)
    {
      return type{std::in_place_index_t<error>{}, std::forward<Ts>(ts)...};
    }

    template <typename NewSuccess>
    struct rebind_success
    {
      using type = Wrapper<NewSuccess, error_type>;
    };

    template <typename NewSuccess>
    using rebind_success_t = typename rebind_success<NewSuccess>::type;
  };

  template <typename Optional>
  struct optional_adapter;

  template <template <typename...> class Optional, typename Value>
  struct optional_adapter<Optional<Value>>
  {
    using type = Optional<Value>;
    using success_type = Value;
    using error_type = std::nullopt_t;
    inline static constexpr auto error_value = std::nullopt;

    template <typename T>
    static bool is_success(T&& t) noexcept
    {
      return t.has_value();
    }

    template <typename T>
    static decltype(auto) get_success(T&& t) noexcept
    {
      return std::forward<T>(t).value();
    }

    template <typename T>
    static decltype(auto) get_error(T&&) noexcept
    {
      return error_value;
    }

    template <typename... Ts>
    static decltype(auto) make_success(Ts&&... ts)
    {
      return type{std::in_place_t{}, std::forward<Ts>(ts)...};
    }

    template <typename... Ts>
    static decltype(auto) make_error(Ts&&... ts)
    {
      return type{error_value};
    }

    template <typename NewValue>
    struct rebind_success
    {
      using type = Optional<NewValue>;
    };
    template <typename NewValue>
    using rebind_success_t = typename rebind_success<NewValue>::type;
  };


  template <template <typename...> class Adapter>
  struct result_interface
  {
    template <typename T>
    struct continuation : crtp<T, continuation>
    {
      using base = crtp<T, continuation>;
      using self_type = continuation;

      //using trait_type = Adapter<typename T::type>;

      bool is_success() const
      { return Adapter<typename T::type>::is_success((*this).that().get()); }

      bool is_error() const
      { return !is_success(); }

      /* TODO make it work
      decltype(auto) success() const &
      { return trait_type::get_success((*this).that()); }

      decltype(auto) success() &
      { return trait_type::get_success((*this).that()); }

      decltype(auto) success() &&
      { return trait_type::get_success(std::move(*this).that()); }

      decltype(auto) error() const &
      { return trait_type::get_error((*this).that()); }

      decltype(auto) error() &
      { return trait_type::get_error((*this).that()); }

      decltype(auto) error() &&
      { return trait_type::get_error(std::move(*this).that()); }
      */

      struct impl
      {
        template <typename ST, typename Alternative>
        static constexpr decltype(auto) success_or(ST&& t, Alternative&& alternative)
        {
          using dST = std::decay_t<ST>;
          using trait = Adapter<typename dST::type>;

          using ReturnType = typename trait::success_type;
          if (trait::is_success(std::forward<ST>(t).get()))
            return ReturnType{trait::get_success(std::forward<ST>(t).get())};
          else
            return ReturnType{std::forward<Alternative>(alternative)};
        }

        template <typename ST, typename Fn>
        static constexpr decltype(auto) map(ST&& t, Fn&& fn)
        {
          using dST = std::decay_t<ST>;
          using trait = Adapter<typename dST::type>;

          // TODO static_assert(std::is_invocable_v<Fn(success_type)>, "Fn must be invocable with success_type");
          //TODO using map_result_type = std::invoke_result_t<Fn(success_type)>;
          using fn_result_type = decltype(fn(std::declval<typename trait::success_type>()));

          using R = typename trait::template rebind_success_t<fn_result_type>;
          using new_trait = Adapter<R>;
          using return_type = rebind_strong_type_t<dST, R>;

          if (trait::is_success(t.get()))
            return return_type(new_trait::make_success(
                  std::invoke(std::forward<Fn>(fn), trait::get_success(std::forward<ST>(t).get()))
                ));
          else
            return return_type(new_trait::make_error(trait::get_error(std::forward<ST>(t).get())));
        }

        template <typename ST, typename Fn, typename Alternative>
        static constexpr decltype(auto) map_or(ST&& t, Fn&& fn, Alternative&& alternative)
        {
          // TODO check whether implementing it can be faster or not
          return success_or(map(std::forward<ST>(t), std::forward<Fn>(fn)), std::forward<Alternative>(alternative));
        }

        template <typename ST, typename Fn>
        static constexpr decltype(auto) and_then(ST&& t, Fn&& fn)
        {
          using dST = std::decay_t<ST>;
          using trait = Adapter<typename dST::type>;

          using fn_result_type = decltype(fn(std::declval<typename trait::success_type>()));

          using R = fn_result_type;
          using new_underlying_type = plain_type_t<R>; // still not totally OK -> underlying type is also a strong type?
          using return_type = rebind_strong_type_t<dST, new_underlying_type>;

          using new_trait = Adapter<typename return_type::type>;
          static_assert(std::is_same_v<typename new_trait::error_type, typename trait::error_type>, "error types must be the same");

          if (trait::is_success(t.get()))
            return return_type(std::invoke(std::forward<Fn>(fn), trait::get_success(std::forward<ST>(t).get())));
          else
            return return_type(new_trait::make_error(trait::get_error(std::forward<ST>(t).get())));
        }

        template <typename ST, typename FnSucc, typename FnErr>
        static constexpr decltype(auto) map_or_else(ST&& t, FnSucc&& fn_succ, FnErr&& fn_err)
        {
          using dST = std::decay_t<ST>;
          using trait = Adapter<typename dST::type>;

          if (trait::is_success(t.get()))
            return std::invoke(std::forward<FnSucc>(fn_succ), trait::get_success(std::forward<ST>(t).get()));
          else
            return std::invoke(std::forward<FnErr>(fn_err), trait::get_error(std::forward<ST>(t).get()));
        }

        template <typename Ret, typename ST, typename FnSucc, typename FnErr>
        static constexpr Ret map_or_else(ST&& t, FnSucc&& fn_succ, FnErr&& fn_err)
        {
          using dST = std::decay_t<ST>;
          using trait = Adapter<typename dST::type>;

          if (trait::is_success(t.get()))
            return static_cast<Ret>(std::invoke(std::forward<FnSucc>(fn_succ), trait::get_success(std::forward<ST>(t).get())));
          else
            return static_cast<Ret>(std::invoke(std::forward<FnErr>(fn_err), trait::get_error(std::forward<ST>(t).get())));
        }

      };

      template <typename Alternative>
      decltype(auto) success_or(Alternative&& alternative) &
      {
        return impl::success_or((*this).that(), std::forward<Alternative>(alternative));
      }

      template <typename Alternative>
      decltype(auto) success_or(Alternative&& alternative) const &
      {
        return impl::success_or((*this).that(), std::forward<Alternative>(alternative));
      }

      template <typename Alternative>
      decltype(auto) success_or(Alternative&& alternative) &&
      {
        return impl::success_or(std::move(*this).that(), std::forward<Alternative>(alternative));
      }

      template <typename Fn, typename Alternative>
      decltype(auto) map_or(Fn&& fn, Alternative&& alternative) &
      {
        return impl::map_or((*this).that(), std::forward<Fn>(fn), std::forward<Alternative>(alternative));
      }

      template <typename Fn, typename Alternative>
      decltype(auto) map_or(Fn&& fn, Alternative&& alternative) const &
      {
        return impl::map_or((*this).that(), std::forward<Fn>(fn), std::forward<Alternative>(alternative));
      }

      template <typename Fn, typename Alternative>
      decltype(auto) map_or(Fn&& fn, Alternative&& alternative) &&
      {
        return impl::map_or(std::move(*this).that(), std::forward<Fn>(fn), std::forward<Alternative>(alternative));
      }

      template <typename Fn>
      constexpr decltype(auto) map(Fn&& fn) &
      {
        return impl::map((*this).that(), std::forward<Fn>(fn));
      }

      template <typename Fn>
      constexpr decltype(auto) map(Fn&& fn) const &
      {
        return impl::map((*this).that(), std::forward<Fn>(fn));
      }

      template <typename Fn>
      constexpr decltype(auto) map(Fn&& fn) &&
      {
        return impl::map(std::move(*this).that(), std::forward<Fn>(fn));
      }

      template <typename FnSucc, typename FnErr>
      constexpr decltype(auto) map_or_else(FnSucc&& fn_succ, FnErr&& fn_err) &
      {
        return impl::map_or_else((*this).that(), std::forward<FnSucc>(fn_succ), std::forward<FnErr>(fn_err));
      }

      template <typename FnSucc, typename FnErr>
      constexpr decltype(auto) map_or_else(FnSucc&& fn_succ, FnErr&& fn_err) const &
      {
        return impl::map_or_else((*this).that(), std::forward<FnSucc>(fn_succ), std::forward<FnErr>(fn_err));
      }

      template <typename FnSucc, typename FnErr>
      constexpr decltype(auto) map_or_else(FnSucc&& fn_succ, FnErr&& fn_err) &&
      {
        return impl::map_or_else(std::move(*this).that(), std::forward<FnSucc>(fn_succ), std::forward<FnErr>(fn_err));
      }

      template <typename Ret, typename FnSucc, typename FnErr>
      constexpr Ret map_or_else(FnSucc&& fn_succ, FnErr&& fn_err) &
      {
        return impl::template map_or_else<Ret>((*this).that(), std::forward<FnSucc>(fn_succ), std::forward<FnErr>(fn_err));
      }

      template <typename Ret, typename FnSucc, typename FnErr>
      constexpr Ret map_or_else(FnSucc&& fn_succ, FnErr&& fn_err) const &
      {
        return impl::template map_or_else<Ret>((*this).that(), std::forward<FnSucc>(fn_succ), std::forward<FnErr>(fn_err));
      }

      template <typename Ret, typename FnSucc, typename FnErr>
      constexpr Ret map_or_else(FnSucc&& fn_succ, FnErr&& fn_err) &&
      {
        return impl::template map_or_else<Ret>(std::move(*this).that(), std::forward<FnSucc>(fn_succ), std::forward<FnErr>(fn_err));
      }


      template <typename Fn>
      constexpr decltype(auto) and_then(Fn&& fn) &
      {
        return impl::and_then((*this).that(), std::forward<Fn>(fn));
      }

      template <typename Fn>
      constexpr decltype(auto) and_then(Fn&& fn) const&
      {
        return impl::and_then((*this).that(), std::forward<Fn>(fn));
      }

      template <typename Fn>
      constexpr decltype(auto) and_then(Fn&& fn) &&
      {
        return impl::and_then(std::move(*this).that(), std::forward<Fn>(fn));
      }
    };
  };

  template <typename T>
  using result_like_continuation = typename result_interface<result_adapter>::continuation<T>;

}

#endif
