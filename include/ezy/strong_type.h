#ifndef STRONG_TYPE_H_INCLUDED
#define STRONG_TYPE_H_INCLUDED

#include <iostream>
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
  }

  template <typename T, typename Tag, template<typename> class... Features>
  class strong_type : public Features<strong_type<T, Tag, Features...>>...
  {
    public:
      using type = T;
      using self_type = strong_type;

      strong_type() = default;

      strong_type(const strong_type&) = default;
      strong_type& operator=(const strong_type&) = default;

      strong_type(strong_type&&) = default;
      strong_type& operator=(strong_type&&) = default;

      template <typename... Args>
      constexpr /*explicit*/ strong_type(Args&&... args
          //, std::enable_if_t<std::is_constructible_v<type, Args...>>* = nullptr
          //, std::enable_if_t<detail::is_braces_constructible<T, Args...>::value>* = nullptr
          //, std::enable_if_t<(sizeof...(Args) != 1) || (!std::is_same_v<ezy::remove_cvref_t<typename detail::headof<Args...>::type>, strong_type>)>* = nullptr
          )
        : _value{std::forward<Args>(args)...}
      {}

      //strong_type(const strong_type& rhs) = default;

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
      T _value;
  };

  // TODO if not already a reference?
  template <typename T, typename Tag, template<typename> class... Features>
  using strong_type_reference = strong_type<typename std::add_lvalue_reference<T>::type, Tag, Features...>;

  template <typename Tag, template <typename> class... Features, typename T>
  auto make_strong(T&& t)
  {
    return strong_type<ezy::remove_cvref_t<T>, Tag, Features...>(std::forward<T>(t));
  }

  template <typename Tag, template <typename> class... Features, typename T>
  auto make_strong_const(T&& t)
  {
    return strong_type<std::add_const_t<ezy::remove_cvref_t<T>>, Tag, Features...>(std::forward<T>(t));
  }

  template <typename T, template<typename> class crtp_type>
  struct crtp
  {
    constexpr T& self() & { return static_cast<T&>(*this); }
    constexpr const T& self() const & { return static_cast<const T&>(*this); }
    constexpr T&& self() && { return static_cast<T&&>(*this); }
  };

  template <typename T, template<typename> class crtp_type>
  struct feature : crtp<T, crtp_type>
  {
    using base = crtp<T, crtp_type>;
    constexpr auto& underlying() & { return base::self().get(); }
    constexpr const auto& underlying() const & { return base::self().get(); }
    constexpr auto&& underlying() && { return std::move(*this).self().get(); }
  };

  template <typename T, template<typename...> class crtp_type>
  struct crtp_with_params
  {
    T& that() { return static_cast<T&>(*this); }
    const T& that() const { return static_cast<const T&>(*this); }
  };
}

#include "tuple_traits.hh" // for contains
namespace ezy
{
  /**
   * strong type traits
   */
  template <typename ST>
  struct is_strong_type : std::false_type
  {};

  template <typename T, typename Tag, template<typename> class... Features>
  struct is_strong_type<strong_type<T, Tag, Features...>> : std::true_type
  {};

  template <typename ST>
  inline constexpr bool is_strong_type_v = is_strong_type<ST>::value;

  template <typename T, typename = void>
  struct plain_type;

  template <typename ST>
  struct plain_type<ST, std::enable_if_t<is_strong_type_v<ST>>>
  {
    using type = typename ST::type;
  };

  template <typename T>
  struct plain_type<T, std::enable_if_t<!is_strong_type_v<T>>>
  {
    using type = T;
  };

  template <typename T>
  using plain_type_t = typename plain_type<T>::type;

  template <typename S>
  struct extract_underlying_type
  {
    static_assert(is_strong_type_v<S>, "is not a strong type");
    using type = typename S::type;
  };

  template <typename T>
  using extract_underlying_type_t = typename extract_underlying_type<T>::type;

  template <typename...>
  struct extract_tag
  {
  };

  template <typename T, typename Tag, template <typename> class... Features>
  struct extract_tag<strong_type<T, Tag, Features...>>
  {
    using type = Tag;
  };

  template <typename... Args>
  using extract_tag_t = typename extract_tag<Args...>::type;

  template <typename...>
  struct extract_features
  {
  };

  template <typename T, typename Tag, template <typename> class... Features>
  struct extract_features<strong_type<T, Tag, Features...>>
  {
    using type = std::tuple<Features<strong_type<T, Tag, Features...>>...>;
  };

  template <typename... Args>
  using extract_features_t = typename extract_features<Args...>::type;

  namespace detail
  {
    template <typename Feature, typename T>
    struct rebind_feature;

    template <template <typename> class Feature, typename ST, typename T>
    struct rebind_feature<Feature<ST>, T>
    {
      using type = Feature<T>;
    };

    template <typename Feature, typename T>
    using rebind_feature_t = typename rebind_feature<Feature, T>::type;
  }

  template <typename ST, template <typename> class... NewFeatures>
  struct rebind_features
  {};

  template <typename T, typename Tag, template <typename> class... OldFeatures, template <typename> class... NewFeatures>
  struct rebind_features<strong_type<T, Tag, OldFeatures...>, NewFeatures...>
  {
    using type = strong_type<T, Tag, NewFeatures...>;
  };

  template <typename ST, template <typename> class... NewFeatures>
  using rebind_features_t = typename rebind_features<ST, NewFeatures...>::type;

  template <typename ST>
  struct strip_strong_type : rebind_features<ST>
  {};

  template <typename ST>
  using strip_strong_type_t = typename strip_strong_type<ST>::type;

  template <typename ST, typename U>
  struct rebind_strong_type
  {};

  template <typename T, typename U, typename Tag, template <typename> class... Features>
  struct rebind_strong_type<strong_type<T, Tag, Features...>, U>
  {
    using type = strong_type<U, Tag, Features...>;
  };

  template <typename ST, typename U>
  using rebind_strong_type_t = typename rebind_strong_type<ST, U>::type;

  /**
   * strong_type_base
   */
  namespace detail
  {
    template <typename T, typename Tag, template <typename> class... Features>
    auto strong_type_base_fn(const ezy::strong_type<T, Tag, Features...>*) -> ezy::strong_type<T, Tag, Features...>;
  }

  template <typename ST>
  struct strong_type_base
  {
    using type = decltype(detail::strong_type_base_fn(std::declval<ST*>()));
  };

  template <typename ST>
  using strong_type_base_t = typename strong_type_base<ST>::type;

  /**
   * has_feature
   */
  namespace detail
  {
    template <typename T>
    struct is_derived_from
    {
      template <typename U>
      struct impl : std::is_base_of<T, U> {};
    };

    struct impersonal {};

    template <typename Feature>
    using impersonalize_t = detail::rebind_feature_t<Feature, impersonal>;

    template <typename ST>
    struct extract_impersonalized_features
    {
      using type = ezy::tuple_traits::map_t< ezy::extract_features_t<ST>, ezy::detail::impersonalize_t>;
    };

    template <typename ST>
    using extract_impersonalized_features_t = typename extract_impersonalized_features<ST>::type;

  }

  template <typename ST, template <typename> class Feature>
  using has_feature =
  ezy::tuple_traits::any_of<
    detail::extract_impersonalized_features_t<ST>,
    detail::is_derived_from<Feature<detail::impersonal>>::template impl
  >;

  template <typename ST, template <typename> class Feature>
  inline constexpr bool has_feature_v = has_feature<ST, Feature>::value;
}

namespace ezy::features
{
  /**
   * features
   */
  template <typename T>
  struct printable : feature<T, printable>
  {
    using base = feature<T, printable>;

    std::ostream& print_to_stream(std::ostream& ostr) const
    {
      return ostr << base::underlying();
    }
  };

  template <typename T, typename Tag, template <typename> class... Features>
  std::ostream& operator<<(std::ostream& ostr, const strong_type<T, Tag, Features...>& strong)
  {
    return strong.print_to_stream(ostr);
  }

  template <typename T>
  struct clonable : feature<T, clonable>
  {
    using base = feature<T, clonable>;

    T clone() const
    {
      return {base::underlying()};
    }
  };

  template <typename T>
  struct implicit_convertible : feature<T, implicit_convertible>
  {
    using base = feature<T, implicit_convertible>;
    using underlying_type = extract_underlying_type_t<T>;

    operator const underlying_type&() const
    {
      return base::underlying();
    }

    operator underlying_type&()
    {
      return base::underlying;
    }
  };
}

namespace ezy
{
  template <typename T>
  struct noncopyable : crtp<T, noncopyable>
  {
    noncopyable() = default;
    noncopyable(const noncopyable&) = delete;
    noncopyable& operator=(const noncopyable&) = delete;
  };

  template <typename T>
  struct nonmovable : crtp<T, nonmovable>
  {
    nonmovable() = default;
    nonmovable(const nonmovable&) = delete;
    nonmovable& operator=(const nonmovable&) = delete;
  };

  template <typename T>
  struct nontransferable : noncopyable<T>, nonmovable<T> {};

  /*
  template <typename...> struct wrapper_trait;

  template <template<typename...> typename Wrapper, typename... Args>
  struct wrapper_trait<Wrapper<Args...>>
  {
    using type = Wrapper<Args...>;

    template <typename... NewArgs>
    using rebind = Wrapper<NewArgs...>;
  }
  */

  using notag_t = void;

  namespace detail
  {
    // for debugging
    template <typename... T>
    struct print_type;
  }
}


#endif
