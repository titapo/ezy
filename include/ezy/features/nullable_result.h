#ifndef EZY_FEATURES_NULLABLE_RESULT_H_INCLUDED
#define EZY_FEATURES_NULLABLE_RESULT_H_INCLUDED

#include "nullable.h"
#include "result_interface.h"

namespace ezy::features::experimental
{
  // This is not a feature
  template <template <typename> class NullableFeature>
  struct adapter_for_nullable
  {
    
    template <typename ValueType>
    struct apply
    {
      // TODO remove_cvref_t
      using success_type = std::decay_t<decltype(NullableFeature<ValueType>::has_value_impl(std::declval<ValueType>()))>;

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
        return NullableFeature<ValueType>::make_null();
      }

      template <typename U>
      using rebind_success_t = U;

    };

  };

  template <typename T>
  struct nullable :
    ezy::features::experimental::default_nullable<T>::apply,
    ezy::features::result_interface
    <
      adapter_for_nullable<ezy::features::experimental::default_nullable>::apply
    >::continuation<T>
  {};

  template <typename T>
  struct nullable_ptr :
    ezy::features::experimental::basic_nullable_ptr<T>,
    ezy::features::result_interface
    <
      adapter_for_nullable<ezy::features::experimental::basic_nullable_ptr>::apply
    >::continuation<T>
  {};
}
#endif

