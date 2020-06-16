#ifndef STRONG_TYPE_H_INCLUDED
#define STRONG_TYPE_H_INCLUDED

#include <utility> // std::forward
#include <type_traits>
#include "type_traits.h"

namespace ezy
{ 
  namespace detail
  {
      template <typename... Types>
      struct identity
      {};

      template <typename T, typename Identity, typename = void>
      struct is_braces_constructible : std::false_type
      {};

      template <typename T, typename... Args>
      struct is_braces_constructible<T, identity<Args...>, std::void_t<decltype(T{std::declval<Args>()...})>> : std::true_type
      {};

      template  <typename... List>
      struct headof;

      template <typename Head, typename... Tail>
      struct headof<Head, Tail...>
      {
        using type = Head;
      };

      template <>
      struct headof<>
      {
        using type = void;
      };

      template <bool Condition>
      using disable_if = std::enable_if<!Condition>;

      template <bool Condition>
      using disable_if_t = std::enable_if_t<!Condition>;

      /**
       * Theoretically the same as std::integral_constant, just deduce the type automatically
       */
      template <auto Const>
      struct constant
      {
        constexpr static auto value = Const;
      };

  }

  using notag_t = void;
  using extended_tag_t = void;

  template <typename T, typename Tag, template<typename> class... Features>
  class strong_type : public Features<strong_type<T, Tag, Features...>>...
  {
    private:
      // introduced as a dependent typedef, to defer evaluation
      template <typename...>
      using is_extended = std::is_same<Tag, extended_tag_t>;

    public:
      using type = T;
      using self_type = strong_type;

      strong_type() = default;

      strong_type(const strong_type&) = default;
      strong_type& operator=(const strong_type&) = default;

      strong_type(strong_type&&) = default;
      strong_type& operator=(strong_type&&) = default;

      template <typename... Args, std::enable_if_t<std::conjunction_v<detail::constant<(sizeof...(Args) > 0)>, std::negation<is_extended<Args...>>>, bool> = true>
      constexpr explicit strong_type(Args&&... args
          //, std::enable_if_t<std::is_constructible_v<type, Args...>>* = nullptr
          //, std::enable_if_t<detail::is_braces_constructible<T, Args...>::value>* = nullptr
          //, std::enable_if_t<(sizeof...(Args) != 1) || (!std::is_same_v<ezy::remove_cvref_t<typename detail::headof<Args...>::type>, strong_type>)>* = nullptr
          )
        : _value{std::forward<Args>(args)...}
      {}

      template <typename... Args, std::enable_if_t<std::conjunction_v<detail::constant<(sizeof...(Args) > 0)>, is_extended<Args...>>, bool> = true>
      constexpr strong_type(Args&&... args)
        : _value{std::forward<Args>(args)...}
      {}

      constexpr T& get() & { return _value; }
      constexpr decltype(auto) get() &&
      {
        if constexpr (std::is_lvalue_reference_v<T>)
          return _value;
        else
          return std::move(_value);
      }
      constexpr const T& get() const & { return _value; }

      explicit operator T() { return _value; }
      explicit operator T() const { return _value; }

      // swap
    private:
      T _value{};
  };

  // TODO if not already a reference?
  template <typename T, typename Tag, template<typename> class... Features>
  using strong_type_reference = strong_type<typename std::add_lvalue_reference<T>::type, Tag, Features...>;

  template <typename Tag, template <typename> class... Features, typename T>
  constexpr auto make_strong(T&& t)
  {
    return strong_type<ezy::remove_cvref_t<T>, Tag, Features...>(std::forward<T>(t));
  }

  template <typename Tag, template <typename> class... Features, typename T>
  constexpr auto make_strong_const(T&& t)
  {
    return strong_type<std::add_const_t<ezy::remove_cvref_t<T>>, Tag, Features...>(std::forward<T>(t));
  }

  template <typename Tag, template <typename> class... Features, typename T>
  constexpr auto make_strong_reference(T&& t)
  {
    return strong_type_reference<std::remove_reference_t<T>, Tag, Features...>(std::forward<T>(t));
  }

  template <typename Tag, template <typename> class... Features, typename T>
  constexpr auto make_strong_reference_const(T&& t)
  {
    static_assert(std::is_lvalue_reference_v<T>, "Cannot form reference to an rvalue!");
    return strong_type_reference<std::add_const_t<std::remove_reference_t<T>>, Tag, Features...>(std::forward<T>(t));
  }

  /**
   * extended_type is a special case: it mainly used for extending the underlying type's
   * interface or capabilities, the tag is mostly irrelevant: it doesn't really matter if two types are
   * accidentally the same. (Currently extended_tag_t is a void, but users should not depend on it.)
   */
  template <typename T, template <typename> class... Features>
  using extended_type = strong_type<T, extended_tag_t, Features...>;

  template <typename T, template <typename> class... Features>
  using extended_type_reference = strong_type_reference<T, extended_tag_t, Features...>;

  template <template <typename> class... Features, typename T>
  constexpr auto make_extended(T&& t)
  {
    return make_strong<extended_tag_t, Features...>(std::forward<T>(t));
  }

  template <template <typename> class... Features, typename T>
  constexpr auto make_extended_const(T&& t)
  {
    return make_strong_const<extended_tag_t, Features...>(std::forward<T>(t));
  }

  template <template <typename> class... Features, typename T>
  constexpr auto make_extended_reference(T&& t)
  {
    return make_strong_reference<extended_tag_t, Features...>(std::forward<T>(t));
  }

  template <template <typename> class... Features, typename T>
  constexpr auto make_extended_reference_const(T&& t)
  {
    return make_strong_reference_const<extended_tag_t, Features...>(std::forward<T>(t));
  }
}

namespace ezy
{
  namespace detail
  {
    // for debugging
    template <typename... T>
    struct print_type;
  }
}


#endif
