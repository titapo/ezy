#ifndef EZY_BITS_OPTIONAL_LIKE_VALUE_H_INCLUDED
#define EZY_BITS_OPTIONAL_LIKE_VALUE_H_INCLUDED

#include <ezy/bits/dependent_forward.h>

namespace ezy
{
  namespace detail
  {
    /**
     * Returns if Range element is referenceable. Element can be accessed through an iterator or by indexing etc.
     *
     * If the Range itself is not an lvalue reference, then its element cannot be referenced.
     * If the returned type is not an lvalue reference, the its element cannot be referenced.
     *
     * The conditions above may not be complete.
     */
    template <typename Range>
    struct is_element_referenceable
    {
      using element_type = decltype(*std::begin(std::declval<Range>())); // preserving const-ness
      static constexpr bool value = std::is_lvalue_reference_v<Range> && std::is_lvalue_reference_v<element_type>;
    };

    template <typename Range>
    static constexpr bool is_element_referenceable_v = is_element_referenceable<Range>::value;

    template <typename Range>
    struct optional_like_value_type
    {
      using value_type = std::remove_reference_t<decltype(*std::begin(std::declval<Range>()))>; // preserving const-ness
      using type = ezy::conditional_t<
        is_element_referenceable_v<Range>,
        const ezy::pointer<value_type>,
        ezy::optional<std::remove_const_t<value_type>>
      >;
    };

    template <typename Range>
    using optional_like_value_type_t = typename optional_like_value_type<Range>::type;

    template <typename Range>
    constexpr auto make_optional_like_value()
    {
      using ReturnType = optional_like_value_type_t<Range>;
      if constexpr (is_element_referenceable_v<Range>)
      {
        return ReturnType{nullptr}; // ezy::pointer has trivial default ctor :/
      }
      else
      {
        return ReturnType{};
      }
    }

    template <typename Range, typename ValueType>
    constexpr auto make_optional_like_value(ValueType&& value)
    {
      using ReturnType = optional_like_value_type_t<Range>;
      auto prepare_argument = [](auto&& argument)
      {
        if constexpr (detail::is_element_referenceable_v<Range>)
          return &argument;
        else
          return std::move(argument);
      };
      return ReturnType(prepare_argument(detail::dependent_forward<Range>(value)));
    }
  }
}

#endif
