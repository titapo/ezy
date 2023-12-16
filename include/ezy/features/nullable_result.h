#ifndef EZY_FEATURES_NULLABLE_RESULT_H_INCLUDED
#define EZY_FEATURES_NULLABLE_RESULT_H_INCLUDED

#include "nullable.h"
#include "result_interface.h"
#include "../type_traits.h"

namespace ezy::features::experimental
{
  // This is not a feature
  template <template <typename> class NullableFeature>
  struct adapter_for_nullable
  {
    
    template <typename ValueType>
    struct apply
    {
      using success_type = ezy::remove_cvref_t<decltype(NullableFeature<ValueType>::has_value_impl(std::declval<ValueType>()))>;

      template <typename T>
      constexpr static bool is_success(T&& t)
      {
        return NullableFeature<ValueType>::has_value_impl(std::forward<T>(t));
      }

      template <typename T>
      constexpr static decltype(auto) get_success(T&& t)
      {
        return NullableFeature<ValueType>::value_impl(std::forward<T>(t)); // dereference
      }

      template <typename T>
      constexpr static decltype(auto) get_error(T&& t)
      {
        return std::forward<T>(t); // ??
      }

      template <typename... Ts>
      constexpr static decltype(auto) make_underlying_success(Ts&&... ts)
      {
        return ValueType{std::forward<Ts>(ts)...};
      }

      template <typename... Ts>
      constexpr static decltype(auto) make_underlying_error(Ts&&...)
      {
        return NullableFeature<ValueType>::make_underlying_null();
      }

      template <typename U>
      using rebind_success_t = U;

    };

  };

  struct nullable
  {
    template <typename T>
    struct impl :
      ezy::features::experimental::default_nullable::impl<T>,
      ezy::features::result_interface
      <
        adapter_for_nullable<ezy::features::experimental::default_nullable::impl>::apply
      >::impl<T>
    {};
  };

  struct nullable_ptr
  {
    template <typename T>
    struct impl :
      ezy::features::experimental::basic_nullable_ptr::impl<T>,
      ezy::features::result_interface
      <
        adapter_for_nullable<ezy::features::experimental::basic_nullable_ptr::impl>::apply
      >::impl<T>,
      ezy::features::operator_arrow::impl<T>
    {};
  };
}
#endif

