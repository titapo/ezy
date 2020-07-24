#ifndef STRONG_TYPE_H_INCLUDED
#define STRONG_TYPE_H_INCLUDED

#include <utility> // std::forward
#include <type_traits>
#include "type_traits.h"

namespace ezy
{ 

  using notag_t = void;
  using extended_tag_t = void;

  namespace detail
  {
    template <typename Tag, typename...>
    using is_tag_extended = std::is_same<Tag, extended_tag_t>;

    template <typename T, bool Extended = false>
    struct strong_type_payload
    {
      strong_type_payload() = default;

      strong_type_payload(const strong_type_payload&) = default;
      strong_type_payload& operator=(const strong_type_payload&) = default;

      strong_type_payload(strong_type_payload&&) = default;
      strong_type_payload& operator=(strong_type_payload&&) = default;

      template <typename Arg0, typename... Args>
      constexpr explicit strong_type_payload(Arg0&& arg0, Args&&... args
          //, std::enable_if_t<std::is_constructible_v<type, Args...>>* = nullptr
          //, std::enable_if_t<detail::is_braces_constructible<T, Args...>::value>* = nullptr
          //, std::enable_if_t<(sizeof...(Args) != 1) || (!std::is_same_v<ezy::remove_cvref_t<typename detail::headof<Args...>::type>, strong_type_payload>)>* = nullptr
          )
        : _value{std::forward<Arg0>(arg0), std::forward<Args>(args)...}
      {}

      T _value{};
    };

    template <typename T>
    struct strong_type_payload<T, true/*extended*/>
    {
      strong_type_payload() = default;

      strong_type_payload(const strong_type_payload&) = default;
      strong_type_payload& operator=(const strong_type_payload&) = default;

      strong_type_payload(strong_type_payload&&) = default;
      strong_type_payload& operator=(strong_type_payload&&) = default;

      template <typename Arg0, typename... Args>
      constexpr strong_type_payload(Arg0&& arg0, Args&&... args)
        : _value{std::forward<Arg0>(arg0), std::forward<Args>(args)...}
      {}

      T _value{};
    };

    template <typename T, bool IsExtended>
    struct strong_type_base : detail::strong_type_payload<T, IsExtended>
    {
      using _payload = detail::strong_type_payload<T, IsExtended>;
      using _payload::strong_type_payload;

      constexpr T& get() & { return this->_value; }
      constexpr decltype(auto) get() &&
      {
        return get_forwarded(std::is_lvalue_reference<T>{});
      }
      constexpr const T& get() const & { return this->_value; }

      explicit operator T() { return this->_value; }
      explicit operator T() const { return this->_value; }

      private:

        // lvalue_reference
        constexpr decltype(auto) get_forwarded(std::true_type)
        {
          return this->_value;
        }

        // not lvalue_reference
        constexpr decltype(auto) get_forwarded(std::false_type)
        {
          return std::move(this->_value);
        }
    };
  }

  template <typename T, typename Tag, template<typename> class... Features>
  class strong_type : public detail::strong_type_base<T, detail::is_tag_extended<Tag>::value>, public Features<strong_type<T, Tag, Features...>>...
  {
    public:
      using type = T;
      using self_type = strong_type;

      using _base = detail::strong_type_base<T, detail::is_tag_extended<Tag>::value>;
      using _base::_base;
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
    static_assert(std::is_lvalue_reference<T>::value, "Cannot form reference to an rvalue!");
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
