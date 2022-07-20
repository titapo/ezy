#ifndef EZY_FEATURES_RESULT_INTERFACE_INCLUDED
#define EZY_FEATURES_RESULT_INTERFACE_INCLUDED

#include "../strong_type_traits.h"
#include "../invoke.h"
#include <ezy/always_false.h>

namespace ezy
{
namespace features
{
  /**
   * Requirements for Adapter<T> (where T is the underlying type):
   *
   * Member functions:
   * static is_success(T) -> bool
   *   Returns: true if T holds success value, otherwise false
   *
   * static get_success(T) -> success_type
   *   Expects: T holds success
   *   Returns: success value
   *
   * static get_error(T) -> error_type
   *   Expects: T does not hold success
   *   Returns: error value
   *
   * static make_underlying_success(Ts&&...) -> T
   *   Constructs a T instance storing success based on Ts...
   *
   * static make_underlying_error(Ts&&...) -> T
   *   Constructs a T instance storing error
   *
   * Member alias templates
   * template <typename U> rebind_success_t
   * template <typename U> rebind_error_t
   *
   * Gives back T' (same wrapper as T, but the changes its success type to U)
   */

  namespace detail
  {
    template <typename Result, typename SourceTrait, typename ResultTrait, typename ST, typename Fn>
    static constexpr Result map(ST&& t, Fn&& fn)
    {
      if (SourceTrait::is_success(t.get()))
        return Result(ResultTrait::make_underlying_success(
              ezy::invoke(std::forward<Fn>(fn), SourceTrait::get_success(std::forward<ST>(t).get()))
            ));
      else
        return Result(ResultTrait::make_underlying_error(SourceTrait::get_error(std::forward<ST>(t).get())));
    }

    template <typename Result, typename SourceTrait, typename ResultTrait, typename ST, typename FnErr>
    static constexpr Result map_error(ST&& t, FnErr&& fn_err)
    {
      if (SourceTrait::is_success(t.get()))
        return Result(ResultTrait::make_underlying_success(SourceTrait::get_success(std::forward<ST>(t).get())));
      else
        return Result(ResultTrait::make_underlying_error(
              ezy::invoke(std::forward<FnErr>(fn_err), SourceTrait::get_error(std::forward<ST>(t).get()))
            ));
    }
  }

  template <template <typename...> class Adapter>
  struct result_interface
  {
    template <typename T>
    struct impl
    {
      template <typename... Ts>
      using result_interface_adapter_t = Adapter<Ts...>;
      //using trait_type = Adapter<typename T::type>;

      bool is_success() const
      { return Adapter<typename T::type>::is_success(static_cast<const T&>(*this).get()); }

      bool is_error() const
      { return !is_success(); }

      decltype(auto) success() const &
      { return Adapter<typename T::type>::get_success(static_cast<const T&>(*this).get()); }

      decltype(auto) success() &
      { return Adapter<typename T::type>::get_success(static_cast<T&>(*this).get()); }

      decltype(auto) success() &&
      { return Adapter<typename T::type>::get_success(static_cast<T&&>(*this).get()); }

      decltype(auto) error() const &
      { return Adapter<typename T::type>::get_error(static_cast<const T&>(*this).get()); }

      decltype(auto) error() &
      { return Adapter<typename T::type>::get_error(static_cast<T&>(*this).get()); }

      decltype(auto) error() &&
      { return Adapter<typename T::type>::get_error(static_cast<T&&>(*this).get()); }

      // TODO it seems those factory functions do not work with non_transferable types, since T (strong type) does not work.
      template <typename... Ts>
      static decltype(auto) make_success(Ts&&... ts)
      {
        return T{Adapter<typename T::type>::make_underlying_success(std::forward<Ts>(ts)...)};
      }

      template <typename... Ts>
      static decltype(auto) make_error(Ts&&... ts)
      {
        return T{Adapter<typename T::type>::make_underlying_error(std::forward<Ts>(ts)...)};
      }

      struct _impl
      {
        template <typename ST, typename Alternative>
        static constexpr decltype(auto) success_or(ST&& t, Alternative&& alternative)
        {
          using dST = ezy::remove_cvref_t<ST>;
          using trait = Adapter<typename dST::type>;

          using ReturnType = typename trait::success_type;
          if (trait::is_success(std::forward<ST>(t).get()))
            return ReturnType{trait::get_success(std::forward<ST>(t).get())};
          else
            return ReturnType{std::forward<Alternative>(alternative)};
        }

        template <typename Result, typename ResultTrait, typename ST, typename Fn>
        static constexpr Result map(ST&& t, Fn&& fn)
        {
          using SourceTrait = Adapter<typename ezy::remove_cvref_t<ST>::type>;
          return detail::map<Result, SourceTrait, ResultTrait>(std::forward<ST>(t), std::forward<Fn>(fn));
        }

        template <typename ST, typename Fn>
        static constexpr decltype(auto) map(ST&& t, Fn&& fn)
        {
          using dST = ezy::remove_cvref_t<ST>;
          using trait = Adapter<typename dST::type>;

          // TODO static_assert(std::is_invocable_v<Fn(success_type)>, "Fn must be invocable with success_type");
          using fn_result_type = decltype(ezy::invoke(fn, std::declval<typename trait::success_type>()));

          using R = typename trait::template rebind_success_t<fn_result_type>;
          using new_trait = Adapter<R>;
          using return_type = rebind_strong_type_t<dST, R>;

          return map<return_type, new_trait>(std::forward<ST>(t), std::forward<Fn>(fn));
        }

        template <typename Result, typename ST, typename Fn>
        static constexpr decltype(auto) map(ST&& t, Fn&& fn)
        {
          //using fn_result_type = decltype(fn(std::declval<typename trait::success_type>()));
          // fn_result_type should be checked if it is convertible to success
          using new_trait = typename Result::template result_interface_adapter_t<typename Result::type>;
          return map<Result, new_trait>(std::forward<ST>(t), std::forward<Fn>(fn));
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
          using dST = ezy::remove_cvref_t<ST>;
          using trait = Adapter<typename dST::type>;

          if constexpr (!ezy::is_invocable<Fn, typename trait::success_type>::value)
          {
            static_assert(ezy::always_false<Fn, typename trait::success_type>, "Fn must be invocable with success_type!");
          }
          using fn_result_type = decltype(ezy::invoke(std::forward<Fn>(fn), std::declval<typename trait::success_type>()));

          using R = fn_result_type;
          using new_underlying_type = plain_type_t<R>; // still not totally OK -> underlying type is also a strong type?
          using return_type = rebind_strong_type_t<dST, new_underlying_type>;

          using new_trait = Adapter<typename return_type::type>;
          static_assert(
              std::is_same<typename new_trait::error_type, typename trait::error_type>::value,
              "error types must be the same"
          );

          if (trait::is_success(t.get()))
            return return_type(ezy::invoke(std::forward<Fn>(fn), trait::get_success(std::forward<ST>(t).get())));
          else
            return return_type(new_trait::make_underlying_error(trait::get_error(std::forward<ST>(t).get())));
        }

        template <typename ST, typename FnSucc, typename FnErr>
        static constexpr decltype(auto) map_or_else(ST&& t, FnSucc&& fn_succ, FnErr&& fn_err)
        {
          using dST = ezy::remove_cvref_t<ST>;
          using trait = Adapter<typename dST::type>;

          if (trait::is_success(t.get()))
            return ezy::invoke(std::forward<FnSucc>(fn_succ), trait::get_success(std::forward<ST>(t).get()));
          else
            return ezy::invoke(std::forward<FnErr>(fn_err), trait::get_error(std::forward<ST>(t).get()));
        }

        template <typename Ret, typename ST, typename FnSucc, typename FnErr>
        static constexpr Ret map_or_else(ST&& t, FnSucc&& fn_succ, FnErr&& fn_err)
        {
          using dST = ezy::remove_cvref_t<ST>;
          using trait = Adapter<typename dST::type>;

          if (trait::is_success(t.get()))
            return static_cast<Ret>(ezy::invoke(std::forward<FnSucc>(fn_succ), trait::get_success(std::forward<ST>(t).get())));
          else
            return static_cast<Ret>(ezy::invoke(std::forward<FnErr>(fn_err), trait::get_error(std::forward<ST>(t).get())));
        }

        template <typename ST, typename FnErr>
        static constexpr decltype(auto) map_error(ST&& t, FnErr&& fn_err)
        {
          using dST = ezy::remove_cvref_t<ST>;
          using trait = Adapter<typename dST::type>;
          using FnErrReturnType = decltype(ezy::invoke(std::forward<FnErr>(fn_err), trait::get_error(std::forward<ST>(t).get())));
          using R = typename trait::template rebind_error_t<FnErrReturnType>;
          using ReturnType = rebind_strong_type_t<dST, R>;
          using ReturnTrait = Adapter<R>;
          return detail::map_error<ReturnType, trait, ReturnTrait>(std::forward<ST>(t), std::forward<FnErr>(fn_err));
        }

        template <typename Ret, typename ST, typename FnErr>
        static constexpr decltype(auto) map_error(ST&& t, FnErr&& fn_err)
        {
          using dST = ezy::remove_cvref_t<ST>;
          using trait = Adapter<typename dST::type>;
          using ReturnTrait = typename Ret::template result_interface_adapter_t<typename Ret::type>;
          return detail::map_error<Ret, trait, ReturnTrait>(std::forward<ST>(t), std::forward<FnErr>(fn_err));
        }

        template <typename ST, typename Fn>
        static constexpr void call_tee(ST&& t, Fn&& fn)
        {
          using Trait = Adapter<typename ezy::remove_cvref_t<ST>::type>;
          if (Trait::is_success(t.get()))
          {
            ezy::invoke(fn, Trait::get_success(t.get()));
          }
        }

      };

      /**
       * success_or(Alternative) -> success_type
       */
      template <typename Alternative>
      decltype(auto) success_or(Alternative&& alternative) &
      {
        return _impl::success_or(static_cast<T&>(*this), std::forward<Alternative>(alternative));
      }

      template <typename Alternative>
      decltype(auto) success_or(Alternative&& alternative) const &
      {
        return _impl::success_or(static_cast<const T&>(*this), std::forward<Alternative>(alternative));
      }

      template <typename Alternative>
      decltype(auto) success_or(Alternative&& alternative) &&
      {
        return _impl::success_or(static_cast<T&&>(*this), std::forward<Alternative>(alternative));
      }

      /**
       * map_or(Fn, Alternative) -> success_type
       */
      template <typename Fn, typename Alternative>
      decltype(auto) map_or(Fn&& fn, Alternative&& alternative) &
      {
        return _impl::map_or(static_cast<T&>(*this), std::forward<Fn>(fn), std::forward<Alternative>(alternative));
      }

      template <typename Fn, typename Alternative>
      decltype(auto) map_or(Fn&& fn, Alternative&& alternative) const &
      {
        return _impl::map_or(static_cast<const T&>(*this), std::forward<Fn>(fn), std::forward<Alternative>(alternative));
      }

      template <typename Fn, typename Alternative>
      decltype(auto) map_or(Fn&& fn, Alternative&& alternative) &&
      {
        return _impl::map_or(static_cast<T&&>(*this), std::forward<Fn>(fn), std::forward<Alternative>(alternative));
      }

      /**
       * map(Fn) -> unspecified
       */
      template <typename Fn>
      constexpr decltype(auto) map(Fn&& fn) &
      {
        return _impl::map(static_cast<T&>(*this), std::forward<Fn>(fn));
      }

      template <typename Fn>
      constexpr decltype(auto) map(Fn&& fn) const &
      {
        return _impl::map(static_cast<const T&>(*this), std::forward<Fn>(fn));
      }

      template <typename Fn>
      constexpr decltype(auto) map(Fn&& fn) &&
      {
        return _impl::map(static_cast<T&&>(*this), std::forward<Fn>(fn));
      }

      /**
       * map<Result>(Fn) -> Result
       */
      template <typename Result, typename Fn>
      constexpr decltype(auto) map(Fn&& fn) &
      {
        return _impl::template map<Result>(static_cast<T&>(*this), std::forward<Fn>(fn));
      }

      template <typename Result, typename Fn>
      constexpr decltype(auto) map(Fn&& fn) const &
      {
        return _impl::template map<Result>(static_cast<const T&>(*this), std::forward<Fn>(fn));
      }

      template <typename Result, typename Fn>
      constexpr decltype(auto) map(Fn&& fn) &&
      {
        return _impl::template map<Result>(static_cast<T&&>(*this), std::forward<Fn>(fn));
      }

      /**
       * map_or_else(FnSuccess, FnError) -> unspecified
       */
      template <typename FnSucc, typename FnErr>
      constexpr decltype(auto) map_or_else(FnSucc&& fn_succ, FnErr&& fn_err) &
      {
        return _impl::map_or_else(static_cast<T&>(*this), std::forward<FnSucc>(fn_succ), std::forward<FnErr>(fn_err));
      }

      template <typename FnSucc, typename FnErr>
      constexpr decltype(auto) map_or_else(FnSucc&& fn_succ, FnErr&& fn_err) const &
      {
        return _impl::map_or_else(static_cast<const T&>(*this), std::forward<FnSucc>(fn_succ), std::forward<FnErr>(fn_err));
      }

      template <typename FnSucc, typename FnErr>
      constexpr decltype(auto) map_or_else(FnSucc&& fn_succ, FnErr&& fn_err) &&
      {
        return _impl::map_or_else(static_cast<T&&>(*this), std::forward<FnSucc>(fn_succ), std::forward<FnErr>(fn_err));
      }

      /**
       * map_or_else<R>(FnSuccess, FnError) -> R
       */
      template <typename Ret, typename FnSucc, typename FnErr>
      constexpr Ret map_or_else(FnSucc&& fn_succ, FnErr&& fn_err) &
      {
        return _impl::template map_or_else<Ret>(static_cast<T&>(*this), std::forward<FnSucc>(fn_succ), std::forward<FnErr>(fn_err));
      }

      template <typename Ret, typename FnSucc, typename FnErr>
      constexpr Ret map_or_else(FnSucc&& fn_succ, FnErr&& fn_err) const &
      {
        return _impl::template map_or_else<Ret>(static_cast<const T&>(*this), std::forward<FnSucc>(fn_succ), std::forward<FnErr>(fn_err));
      }

      template <typename Ret, typename FnSucc, typename FnErr>
      constexpr Ret map_or_else(FnSucc&& fn_succ, FnErr&& fn_err) &&
      {
        return _impl::template map_or_else<Ret>(static_cast<T&&>(*this), std::forward<FnSucc>(fn_succ), std::forward<FnErr>(fn_err));
      }

      /**
       * map_error(FnError: E1 -> E2) -> <T, E2>
       */
      template <typename FnErr>
      constexpr decltype(auto) map_error(FnErr&& fn_err) &
      {
        return _impl::map_error(static_cast<T&>(*this), std::forward<FnErr>(fn_err));
      }

      template <typename FnErr>
      constexpr decltype(auto) map_error(FnErr&& fn_err) const &
      {
        return _impl::map_error(static_cast<const T&>(*this), std::forward<FnErr>(fn_err));
      }

      template <typename FnErr>
      constexpr decltype(auto) map_error(FnErr&& fn_err) &&
      {
        return _impl::map_error(static_cast<T&&>(*this), std::forward<FnErr>(fn_err));
      }

      /**
       * map_error<Ret>(FnError: E1 -> E2) -> Ret<T, E2>
       */
      template <typename Ret, typename FnErr>
      constexpr decltype(auto) map_error(FnErr&& fn_err) &
      {
        return _impl::template map_error<Ret>(static_cast<T&>(*this), std::forward<FnErr>(fn_err));
      }

      template <typename Ret, typename FnErr>
      constexpr decltype(auto) map_error(FnErr&& fn_err) const &
      {
        return _impl::template map_error<Ret>(static_cast<const T&>(*this), std::forward<FnErr>(fn_err));
      }

      // test needed
      template <typename Ret, typename FnErr>
      constexpr decltype(auto) map_error(FnErr&& fn_err) &&
      {
        return _impl::template map_error<Ret>(static_cast<T&&>(*this), std::forward<FnErr>(fn_err));
      }

      /**
       * and_then(Fn) -> unspecified
       */
      template <typename Fn>
      constexpr decltype(auto) and_then(Fn&& fn) &
      {
        return _impl::and_then(static_cast<T&>(*this), std::forward<Fn>(fn));
      }

      template <typename Fn>
      constexpr decltype(auto) and_then(Fn&& fn) const&
      {
        return _impl::and_then(static_cast<const T&>(*this), std::forward<Fn>(fn));
      }

      template <typename Fn>
      constexpr decltype(auto) and_then(Fn&& fn) &&
      {
        return _impl::and_then(static_cast<T&&>(*this), std::forward<Fn>(fn));
      }

      template <typename Fn>
      constexpr decltype(auto) tee(Fn&& fn) const&
      {
        _impl::call_tee(static_cast<const T&>(*this), std::forward<Fn>(fn));
        return *this;
      }
    };
  };
}
}

#endif
