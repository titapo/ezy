#ifndef EZY_STRONG_TYPE_TRAITS_H_INCLUDED
#define EZY_STRONG_TYPE_TRAITS_H_INCLUDED

#include "strong_type.h"
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


#endif
