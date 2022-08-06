#ifndef EZY_TYPELIST_TRAITS_HH_INCLUDED
#define EZY_TYPELIST_TRAITS_HH_INCLUDED

#include <type_traits>
#include <utility>

#include "type_traits.h"

namespace ezy
{
  template <typename...Ts>
  struct typelist {};
}

namespace ezy { namespace tuple_traits
{
  /**
   * append
   */
  template <typename Tuple, typename NewElementType>
  struct append;

  template <template <typename...> class Tuple, typename... Ts, typename NewElementType>
  struct append<Tuple<Ts...>, NewElementType>
  {
    using type = Tuple<Ts..., NewElementType>;
  };

  template <typename Tuple, typename NewElementType>
  using append_t = typename append<Tuple, NewElementType>::type;

  /**
   * extend
   */
  namespace detail
  {
    template <typename...>
    struct combine_tuple_like;

    template <typename TupleLike>
    struct combine_tuple_like<TupleLike>
    {
      using type = TupleLike;
    };

    template <template <typename...> class Tuple, typename... Ts1, typename... Ts2, typename...Tuples>
    struct combine_tuple_like<Tuple<Ts1...>, Tuple<Ts2...>, Tuples...>
    {
      using type = typename combine_tuple_like<Tuple<Ts1..., Ts2...>,  Tuples...>::type;
    };
  }
  template <typename... Tuples>
  struct extend
  {
    using type = typename detail::combine_tuple_like<Tuples...>::type;
  };

  template <typename... Tuples>
  using extend_t = typename extend<Tuples...>::type;

  /**
   * head
   */
  template <typename Tuple>
  struct head;

  template <template <typename...> class Tuple, typename Head, typename... Tail>
  struct head<Tuple<Head, Tail...>>
  {
    using type = Head;
  };

  template <typename Tuple>
  using head_t = typename head<Tuple>::type;

  /**
   * head
   */
  template <typename Tuple>
  struct tail;

  template <template <typename...> class Tuple, typename Head, typename... Tail>
  struct tail<Tuple<Head, Tail...>>
  {
    using type = Tuple<Tail...>;
  };

  template <typename Tuple>
  using tail_t = typename tail<Tuple>::type;

  /**
   * remove
   */
  template <typename T, typename... Ts>
  struct tuple_remove_helper;

  template <template <typename...> class Tuple, typename T>
  struct tuple_remove_helper<T, Tuple<>>
  {
    using type = Tuple<>;
  };

  template <template <typename...> class Tuple, typename Head, typename... Tail, typename T>
  struct tuple_remove_helper<T, Tuple<Head, Tail...>>
  {
    using type = extend_t<
                          ezy::conditional_t<
                            !std::is_same<Head, T>::value,
                            Tuple<Head>,
                            Tuple<>
                          >,
                          typename tuple_remove_helper<T, Tuple<Tail...>>::type
                         >;
  };

  template <typename Tuple, typename T>
  struct remove
  {
    using type = typename tuple_remove_helper<T, Tuple>::type;
  };

  template <typename Tuple, typename T>
  using remove_t = typename remove<Tuple, T>::type;

  /**
   * any_of
   */
  template <typename Tuple, template <typename> class Predicate>
  struct any_of;

  template <template <typename...> class Tuple, template <typename> class Predicate>
  struct any_of<Tuple<>, Predicate> : std::false_type {};

  template <template <typename...> class Tuple, template <typename> class Predicate, typename Head, typename... Tail>
  struct any_of<Tuple<Head, Tail...>, Predicate> :
    ezy::conditional_t<
      Predicate<Head>::value,
      std::true_type,
      any_of<Tuple<Tail...>, Predicate>
    >
  {};

  template <typename Tuple, template <typename> class Predicate>
  constexpr bool any_of_v = any_of<Tuple, Predicate>::value;

  /**
   * none_of
   */
  template <typename Tuple, template <typename> class Predicate>
  struct none_of;

  template <template <typename...> class Tuple, template <typename> class Predicate>
  struct none_of<Tuple<>, Predicate> : std::true_type {};

  template <template <typename...> class Tuple, template <typename> class Predicate, typename Head, typename... Tail>
  struct none_of<Tuple<Head, Tail...>, Predicate> :
    ezy::conditional_t<
      Predicate<Head>::value,
      std::false_type,
      none_of<Tuple<Tail...>, Predicate>
    >
  {};

  template <typename Tuple, template <typename> class Predicate>
  constexpr bool none_of_v = none_of<Tuple, Predicate>::value;

  /**
   * all_of
   */

  template <typename Tuple, template <typename> class Predicate>
  struct all_of;

  template <template <typename...> class Tuple, template <typename> class Predicate>
  struct all_of<Tuple<>, Predicate> : std::true_type {};

  template <template <typename...> class Tuple, template <typename> class Predicate, typename Head, typename... Tail>
  struct all_of<Tuple<Head, Tail...>, Predicate> :
    ezy::conditional_t<
      !Predicate<Head>::value,
      std::false_type,
      all_of<Tuple<Tail...>, Predicate>
    >
  {};

  template <typename Tuple, template <typename> class Predicate>
  constexpr bool all_of_v = all_of<Tuple, Predicate>::value;


  /**
   * contains
   */

  template <typename Tuple, typename T>
  struct contains;

  template <template <typename...> class Tuple, typename T>
  struct contains<Tuple<>, T> : std::false_type {};

  template <template <typename...> class Tuple, typename T, typename Head, typename... Tail>
  struct contains<Tuple<Head, Tail...>, T> : contains<Tuple<Tail...>, T> {};

  template <template <typename...> class Tuple, typename T, typename... Tail>
  struct contains<Tuple<T, Tail...>, T> : std::true_type {};

  template <typename Tuple, typename T>
  constexpr bool contains_v = contains<Tuple, T>::value;

  /**
   * filter
   */
  template <typename Tuple, template <typename> class Predicate>
  struct filter;

  template <template <typename...> class Tuple, template <typename> class Predicate>
  struct filter<Tuple<>, Predicate>
  {
    using type = Tuple<>;
  };

  template <template <typename...> class Tuple, typename Head, typename... Tail, template <typename> class Predicate>
  struct filter<Tuple<Head, Tail...>, Predicate>
  {
    using type = extend_t<
        ezy::conditional_t<
          Predicate<Head>::value,
          Tuple<Head>,
          Tuple<>
        >,
        typename filter<Tuple<Tail...>, Predicate>::type
      >;
  };

  template <typename Tuple, template <typename> class Predicate>
  using filter_t = typename filter<Tuple, Predicate>::type;

  /**
   * unique
   */
  template <typename Tuple>
  struct unique;

  template <template <typename...> class Tuple>
  struct unique<Tuple<>>
  {
    using type = Tuple<>;
  };

  template <template <typename...> class Tuple, typename Head, typename... Tail>
  struct unique<Tuple<Head, Tail...>>
  {
    using type = extend_t<
                   Tuple<Head>,
                   typename unique<
                     remove_t<Tuple<Tail...>, Head>
                   >::type
                 >;
  };

  template <typename Tuple>
  using unique_t = typename unique<Tuple>::type;

  /**
   * flatten
   */
  template <typename TupleOfTuples>
  struct flatten;

  template <template <typename...>class Tuple, typename... Tuples>
  struct flatten<Tuple<Tuples...>>
  {
    using type = extend_t<Tuples...>;
  };

  template <typename TupleOfTuples>
  using flatten_t = typename flatten<TupleOfTuples>::type;


  /**
   * subtract
   */
  template <typename Tuple1, typename Tuple2>
  struct subtract;

  template <typename Tuple1, template <typename...> class Tuple>
  struct subtract<Tuple1, Tuple<>>
  {
    using type = Tuple1;
  };

  template <template <typename...> class Tuple, typename Tuple1, typename Tuple2Head, typename... Tuple2Tail>
  struct subtract<Tuple1, Tuple<Tuple2Head, Tuple2Tail...>>
  {
    using type = typename subtract<
                                   remove_t<Tuple1, Tuple2Head>,
                                   Tuple<Tuple2Tail...>
                                  >::type;
  };

  template <typename Tuple1, typename Tuple2>
  using subtract_t = typename subtract<Tuple1, Tuple2>::type;

  /**
   * map
   */
  template <typename Tuple, template <typename> class Metafunction>
  struct map;

  template <template <typename...> class Tuple, typename... TupleArgs, template <typename> class Metafunction>
  struct map<Tuple<TupleArgs...>, Metafunction>
  {
    using type = Tuple< Metafunction<TupleArgs>... >;
  };

  template <typename Tuple, template <typename> class Metafunction>
  using map_t = typename map<Tuple, Metafunction>::type;

  /**
   * fold
   */
  template <typename Tuple, typename Init, template <typename, typename> class Op>
  struct fold;

  template <template <typename...> class Tuple, typename Init, template <typename, typename> class Op>
  struct fold<Tuple<>, Init, Op>
  {
    using type = Init;
  };

  template <
    template <typename...> class Tuple,
    typename Head, typename... Tail,
    typename Init,
    template <typename, typename> class Op
  >
  struct fold<Tuple<Head, Tail...>, Init, Op>
  {
    using type = typename fold<Tuple<Tail...>, Op<Init, Head>, Op>::type;
  };

  template <typename Tuple, typename Init, template <typename, typename> class Op>
  using fold_t = typename fold<Tuple, Init, Op>::type;


  /**
   * zip
   */
  template <typename Tuple1, typename Tuple2>
  struct zip;

  template <template <typename...> class Tuple>
  struct zip<Tuple<>, Tuple<>>
  {
    using type = Tuple<>;
  };

  template <template <typename...> class Tuple, typename Head1, typename... Tail1, typename Head2, typename... Tail2>
  struct zip<Tuple<Head1, Tail1...>, Tuple<Head2, Tail2...>>
  {
    static_assert(
        sizeof...(Tail1) == sizeof...(Tail2),
        "zipped tuples must have the same size");

    using type = extend_t<
      Tuple<Tuple<Head1, Head2>>,
      typename zip<Tuple<Tail1...>, Tuple<Tail2...>>::type
    >;
  };

  template <typename Tuple1, typename Tuple2>
  using zip_t = typename zip<Tuple1, Tuple2>::type;

  /**
   * enumerate
   */

  namespace detail
  {
    template <template <typename...> class Typelist, typename IndexSequence>
    struct to_typelist;

    template <template <typename...> class Typelist, typename T, T... Is>
    struct to_typelist<Typelist, std::integer_sequence<T, Is...>>
    {
      using type = Typelist<std::integral_constant<T, Is>...>;
    };

    template <template <typename...> class Typelist, typename IndexSequence>
    using to_typelist_t = typename to_typelist<Typelist, IndexSequence>::type;
  }

  template <typename Tuple>
  struct enumerate;

  template <template <typename...> class Tuple, typename... Ts>
  struct enumerate<Tuple<Ts...>>
  {
    using type = zip_t<
      detail::to_typelist_t<Tuple, std::index_sequence_for<Ts...>>,
      Tuple<Ts...>
    >;
  };

  template <typename Tuple>
  using enumerate_t = typename enumerate<Tuple>::type;

  /**
   * rebind
   */
  template <typename T, template <typename...> class U>
  struct rebind;

  template <template <typename...> class Outer, typename... Args, template <typename...> class U>
  struct rebind<Outer<Args...>, U>
  { using type = U<Args...>; };

  template <typename T, template <typename...> class U>
  using rebind_t = typename rebind<T, U>::type;


  /**
   * extract
   */
  template <typename T>
  struct extract : rebind<T, ezy::typelist> {};

  template <typename T>
  using extract_t = typename extract<T>::type;

}} // ezy::tuple_traits

#endif
