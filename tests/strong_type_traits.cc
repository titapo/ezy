#include <catch.hpp>

#include <ezy/strong_type>

/**
 * strong type traits
 */

SCENARIO("compilation tests")
{
  using Simple = ezy::strong_type<int, struct Tag>;
  using SimpleDouble = ezy::strong_type<double, struct Tag>;
  using SimpleRef = ezy::strong_type_reference<int, struct Tag>;
  using OneFeature = ezy::strong_type<int, struct Tag, ezy::features::addable>;
  using MoreFeatures = ezy::strong_type<int, struct Tag, ezy::features::addable, ezy::features::subtractable, ezy::features::equal_comparable>;
  using ComposedFeature = ezy::strong_type<int, struct Tag, ezy::features::additive>;

  static_assert(ezy::is_strong_type_v<int> == false);
  static_assert(ezy::is_strong_type_v<Simple> == true);
  static_assert(ezy::is_strong_type_v<SimpleRef> == true);
  static_assert(ezy::is_strong_type_v<OneFeature> == true);
  static_assert(ezy::is_strong_type_v<MoreFeatures> == true);

  static_assert(std::is_same_v<ezy::plain_type_t<int>, int>);
  static_assert(std::is_same_v<ezy::plain_type_t<Simple>, int>);
  static_assert(std::is_same_v<ezy::plain_type_t<SimpleRef>, int&>);
  static_assert(std::is_same_v<ezy::plain_type_t<OneFeature>, int>);
  static_assert(std::is_same_v<ezy::plain_type_t<MoreFeatures>, int>);

  //static_assert(std::is_same_v<ezy::extract_underlying_type_t<int>, int>); // must fail
  static_assert(std::is_same_v<ezy::extract_underlying_type_t<Simple>, int>);
  static_assert(std::is_same_v<ezy::extract_underlying_type_t<SimpleRef>, int&>);
  static_assert(std::is_same_v<ezy::extract_underlying_type_t<OneFeature>, int>);
  static_assert(std::is_same_v<ezy::extract_underlying_type_t<MoreFeatures>, int>);

  //static_assert(std::is_same_v<extract_tag_t<int>, struct Tag>); // must fail
  static_assert(std::is_same_v<ezy::extract_tag_t<Simple>, struct Tag>);
  static_assert(std::is_same_v<ezy::extract_tag_t<SimpleRef>, struct Tag>);
  static_assert(std::is_same_v<ezy::extract_tag_t<OneFeature>, struct Tag>);
  static_assert(std::is_same_v<ezy::extract_tag_t<MoreFeatures>, struct Tag>);

  static_assert(std::is_same_v<ezy::extract_features_t<Simple>, std::tuple<>>);
  static_assert(std::is_same_v<ezy::extract_features_t<SimpleRef>, std::tuple<>>);
  static_assert(std::is_same_v<ezy::extract_features_t<OneFeature>, std::tuple<ezy::features::addable>>);
  static_assert(std::is_same_v<ezy::extract_features_t<MoreFeatures>, std::tuple<ezy::features::addable, ezy::features::subtractable, ezy::features::equal_comparable>>);

  static_assert(std::is_same_v<ezy::strip_strong_type_t<Simple>, Simple>);
  static_assert(std::is_same_v<ezy::strip_strong_type_t<SimpleRef>, SimpleRef>);
  static_assert(std::is_same_v<ezy::strip_strong_type_t<OneFeature>, Simple>);
  static_assert(std::is_same_v<ezy::strip_strong_type_t<MoreFeatures>, Simple>);

  static_assert(std::is_same_v<ezy::rebind_strong_type_t<Simple, double>, ezy::strong_type<double, struct Tag>>);
  static_assert(std::is_same_v<ezy::rebind_strong_type_t<SimpleRef, double>, ezy::strong_type<double, struct Tag>>); // TODO think: not a estrong_type_reference
  static_assert(std::is_same_v<ezy::rebind_strong_type_t<OneFeature, double>, ezy::strong_type<double, struct Tag, ezy::features::addable>>);
  static_assert(std::is_same_v<ezy::rebind_strong_type_t<MoreFeatures, double>, ezy::strong_type<double, struct Tag, ezy::features::addable, ezy::features::subtractable, ezy::features::equal_comparable>>);

  static_assert(ezy::has_feature_v<Simple, ezy::features::addable> == false);
  static_assert(ezy::has_feature_v<SimpleRef, ezy::features::addable> == false);
  static_assert(ezy::has_feature_v<OneFeature, ezy::features::addable> == true);
  static_assert(ezy::has_feature_v<OneFeature, ezy::features::equal_comparable> == false);
  static_assert(ezy::has_feature_v<MoreFeatures, ezy::features::equal_comparable> == true);
  // (eg. derived/composed features)
  // It may be nicer to check those requirements as concepts (structured), but there is a way to support named
  // features
  static_assert(ezy::has_feature_v<ComposedFeature, ezy::features::equal_comparable> == false);
  static_assert(ezy::has_feature_v<ComposedFeature, ezy::features::addable> == true);
  static_assert(ezy::has_feature_v<ComposedFeature, ezy::features::additive> == true);

  static_assert(std::is_same_v<ezy::rebind_features_t<Simple>, Simple>);
  static_assert(std::is_same_v<ezy::rebind_features_t<OneFeature>, Simple>);
  static_assert(std::is_same_v<ezy::rebind_features_t<MoreFeatures>, Simple>);

  static_assert(std::is_same_v<ezy::rebind_features_t<Simple, ezy::features::addable>, OneFeature>);
  static_assert(std::is_same_v<ezy::rebind_features_t<OneFeature, ezy::features::addable>, OneFeature>);
  static_assert(std::is_same_v<ezy::rebind_features_t<MoreFeatures, ezy::features::addable>, OneFeature>);

  static_assert(std::is_same_v<
      ezy::rebind_features_from_tuple_t<Simple, std::tuple<ezy::features::addable>>,
      OneFeature
      >);

  static_assert(std::is_same_v<
      ezy::rebind_features_from_other_t<SimpleDouble, MoreFeatures>,
      ezy::strong_type<double, struct Tag, ezy::features::addable, ezy::features::subtractable, ezy::features::equal_comparable>
      >);
}


SCENARIO("derived strong type")
{
  struct DerivedSimple : ezy::strong_type<int, DerivedSimple, ezy::features::addable>
  {
    using strong_type::strong_type;
  };

  static_assert(!ezy::is_strong_type_v<DerivedSimple>); //it fails // should be accepted?
  static_assert(ezy::is_strong_type_v<ezy::strong_type_base_t<DerivedSimple>>);

  static_assert(std::is_same_v<ezy::strong_type_base_t<DerivedSimple>, ezy::strong_type<int, DerivedSimple, ezy::features::addable>>);
  static_assert(std::is_same_v<ezy::strong_type_base_t<ezy::strong_type_base_t<DerivedSimple>>, ezy::strong_type<int, DerivedSimple, ezy::features::addable>>);

  DerivedSimple s{3};

  REQUIRE((s + s).get() == 6);
}

