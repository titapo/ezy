#ifndef EZY_STRONG_TYPE_TRAITS_H_INCLUDED
#define EZY_STRONG_TYPE_TRAITS_H_INCLUDED

#include "strong_type.h"
#include "tuple_traits.hh" // for contains
#include "type_traits.h"

namespace ezy
{
  /**
   * strong type traits
   */
  template <typename ST>
  struct is_strong_type : std::false_type
  {};

  template <typename T, typename Tag, typename... Features>
  struct is_strong_type<strong_type<T, Tag, Features...>> : std::true_type
  {};

  template <typename ST>
  constexpr bool is_strong_type_v = is_strong_type<ST>::value;

  template <typename S>
  struct extract_underlying_type;

  template <typename T, typename Tag, typename... Features>
  struct extract_underlying_type<strong_type<T, Tag, Features...>>
  {
    using type = T;
  };

  template <typename T>
  using extract_underlying_type_t = typename extract_underlying_type<T>::type;


  template <typename T>
  struct plain_type
  {
    using type = typename std::conditional_t<is_strong_type_v<T>,
            extract_underlying_type<T>,
            type_identity<T>
          >::type;
  };

  template <typename T>
  using plain_type_t = typename plain_type<T>::type;

  template <typename...>
  struct extract_tag
  {
  };

  template <typename T, typename Tag, typename... Features>
  struct extract_tag<strong_type<T, Tag, Features...>>
  {
    using type = Tag;
  };

  template <typename... Args>
  using extract_tag_t = typename extract_tag<Args...>::type;

  /**
   * extract_features
   */
  template <typename...>
  struct extract_features
  {
  };

  template <typename T, typename Tag, typename... Features>
  struct extract_features<strong_type<T, Tag, Features...>>
  {
    using type = std::tuple<Features...>;
  };

  template <typename... Args>
  using extract_features_t = typename extract_features<Args...>::type;

  /**
   * rebind_features
   */
  template <typename ST, typename... NewFeatures>
  struct rebind_features
  {};

  template <typename T, typename Tag, typename... OldFeatures, typename... NewFeatures>
  struct rebind_features<strong_type<T, Tag, OldFeatures...>, NewFeatures...>
  {
    using type = strong_type<T, Tag, NewFeatures...>;
  };

  template <typename ST, typename... NewFeatures>
  using rebind_features_t = typename rebind_features<ST, NewFeatures...>::type;

  /**
   * rebind_features_from_tuple
   */
  template <typename ST, typename FeaturesTuple>
  struct rebind_features_from_tuple {};

  template <typename ST, typename... Features>
  struct rebind_features_from_tuple<ST, std::tuple<Features...>>
  {
    using type = ezy::rebind_features_t<ST, Features...>;
  };

  template <typename ST, typename FeaturesTuple>
  using rebind_features_from_tuple_t = typename rebind_features_from_tuple<ST, FeaturesTuple>::type;

  /**
   * rebind_features_from_tuple<ST1, ST2>
   *
   * where ST1 = strong_type<T1, Tag1, F1a, F1b>
   * and ST2 = strong_type<T2, Tag2, F2a, F2b>
   *
   * returns: strong_type<T1, Tag1, F2a, F2b>
   */

  template <typename ST, typename OtherST>
  struct rebind_features_from_other
  {
    using type = rebind_features_from_tuple_t<ST, extract_features_t<OtherST>>;
  };

  template <typename ST, typename OtherST>
  using rebind_features_from_other_t = typename rebind_features_from_other<ST, OtherST>::type;

  /**
   * strip_strong_type<ST>
   *
   * Strips features from `ST`
   * Returns: strong_type<T, Tag>
   */

  template <typename ST>
  struct strip_strong_type : rebind_features<ST>
  {};

  template <typename ST>
  using strip_strong_type_t = typename strip_strong_type<ST>::type;

  /**
   * rebind_strong_type
   *
   * rebinds underlying type
   */
  template <typename ST, typename U>
  struct rebind_strong_type
  {};

  template <typename T, typename U, typename Tag, typename... Features>
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
    template <typename T, typename Tag, typename... Features>
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
    // impersonalization is no longer required
    template <typename T>
    struct is_derived_from
    {
      template <typename U>
      struct apply : std::is_base_of<T, U> {};
    };

    struct impersonal {};

    template <typename Feature>
    using impersonalize_t = impl_t<Feature, impersonal>;

    template <typename ST>
    struct extract_impersonalized_features
    {
      using type = ezy::tuple_traits::map_t< ezy::extract_features_t<ST>, ezy::detail::impersonalize_t>;
    };

    template <typename ST>
    using extract_impersonalized_features_t = typename extract_impersonalized_features<ST>::type;

  }

  template <typename ST, typename Feature>
  using has_feature =
  ezy::tuple_traits::any_of<
    detail::extract_impersonalized_features_t<ST>,
    detail::is_derived_from<detail::impersonalize_t<Feature>>::template apply
  >;

  template <typename ST, typename Feature>
  constexpr bool has_feature_v = has_feature<ST, Feature>::value;
}


#endif
