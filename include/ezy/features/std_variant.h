#ifndef EZY_FEATURES_STD_VARIANT_H_INCLUDED
#define EZY_FEATURES_STD_VARIANT_H_INCLUDED

#include "../strong_type.h"
#include "result_interface.h"
#include <cstddef>
#include <variant>

namespace ezy::features
{
  /**
   * features::result_interface adapter for variant<Success, Error>.
   */

  template <typename Type>
  struct result_adapter;

  template <template <typename...> class Wrapper, typename Success, typename Error>
  struct result_adapter<Wrapper<Success, Error>>
  {
    using type = Wrapper<Success, Error>;
    inline static constexpr size_t success = 0;
    inline static constexpr size_t error = 1;
    using success_type = ezy::remove_cvref_t<decltype(std::get<success>(std::declval<type>()))>;
    using error_type = ezy::remove_cvref_t<decltype(std::get<error>(std::declval<type>()))>;

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
    static decltype(auto) make_underlying_success(Ts&&... ts)
    {
      return type{std::in_place_index_t<success>{}, std::forward<Ts>(ts)...};
    }

    template <typename... Ts>
    static decltype(auto) make_underlying_error(Ts&&... ts)
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

    template <typename NewError>
    struct rebind_error
    {
      using type = Wrapper<success_type, NewError>;
    };

    template <typename NewError>
    using rebind_error_t = typename rebind_error<NewError>::type;
  };

  struct result_like_continuation
  {
    template <typename T>
    using impl = typename result_interface<result_adapter>::impl<T>;
  };

}

#endif


