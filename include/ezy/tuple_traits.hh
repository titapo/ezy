#ifndef EZY_TUPLE_TRAITS_HH_INCLUDED
#define EZY_TUPLE_TRAITS_HH_INCLUDED

#include <tuple>
#include <type_traits>

namespace ezy::tuple_traits
{
  template <typename>
  struct is_tuple: std::false_type {};

  template <typename... T>
  struct is_tuple<std::tuple<T...>>: std::true_type {};

  template <typename T>
  constexpr bool is_tuple_v = is_tuple<T>::value;

  /**
   * tuplify
   *
   *   tuplify makes a tuple
   *    tuple -> tuple (no change)
   *    void -> tuple<> (empty)
   *    other -> tuple<other>
   *
   */
  template <typename T, typename = void>
  struct tuplify
  {
    using type = std::tuple<T>;
  };

  template <typename T>
  struct tuplify<T, std::enable_if_t<is_tuple_v<T>>>
  {
    using type = T;
  };

  template <typename T>
  struct tuplify<T, std::enable_if_t<std::is_same_v<T, void>>>
  {
    using type = std::tuple<>;
  };

  template <typename T>
  using tuplify_t = typename tuplify<T>::type;


  /**
   * append
   */
  template <typename Tuple, typename NewElementType>
  struct append
  {
    static_assert(is_tuple_v<Tuple>, "it must be a tuple");

    template <typename NewElement, typename...Args>
    static std::tuple<Args..., NewElement> helper(std::tuple<Args...>, NewElement);

    using type = decltype(helper(std::declval<Tuple>(), std::declval<NewElementType>()));
  };

  template <typename Tuple, typename NewElementType>
  using append_t = typename append<Tuple, NewElementType>::type;

  /**
   * extend
   */
  template <typename... Tuples>
  struct extend
  {
    using type = decltype(std::tuple_cat(std::declval<Tuples>()...));
  };

  template <typename... Tuples>
  using extend_t = typename extend<Tuples...>::type;

  /**
   * head
   */
  template <typename Tuple>
  struct head;

  template <typename Head, typename... Tail>
  struct head<std::tuple<Head, Tail...>>
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

  template <typename Head, typename... Tail>
  struct tail<std::tuple<Head, Tail...>>
  {
    using type = std::tuple<Tail...>;
  };

  template <typename Tuple>
  using tail_t = typename tail<Tuple>::type;

  /**
   * remove
   */
  template <typename T, typename... Ts>
  struct tuple_remove_helper;

  template <typename T>
  struct tuple_remove_helper<T, std::tuple<>>
  {
    using type = std::tuple<>;
  };

  template <typename T, typename Head, typename... Tail>
  struct tuple_remove_helper<T, std::tuple<Head, Tail...>>
  {
    using type = extend_t<
                          std::conditional_t<
                            !std::is_same_v<Head, T>,
                            std::tuple<Head>,
                            std::tuple<>
                          >,
                          typename tuple_remove_helper<T, std::tuple<Tail...>>::type
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

  template <template <typename> class Predicate>
  struct any_of<std::tuple<>, Predicate> : std::false_type {};

  template <template <typename> class Predicate, typename Head, typename... Tail>
  struct any_of<std::tuple<Head, Tail...>, Predicate> :
    std::conditional_t<
      Predicate<Head>::value,
      std::true_type,
      any_of<std::tuple<Tail...>, Predicate>
    >
  {};

  template <typename Tuple, template <typename> class Predicate>
  inline constexpr bool any_of_v = any_of<Tuple, Predicate>::value;

  /**
   * contains
   */
  template <typename Tuple, typename T>
  struct contains;

  template <typename T>
  struct contains<std::tuple<>, T> : std::false_type {};

  template <typename T, typename Head, typename... Tail>
  struct contains<std::tuple<Head, Tail...>, T> : contains<std::tuple<Tail...>, T> {};

  template <typename T, typename... Tail>
  struct contains<std::tuple<T, Tail...>, T> : std::true_type {};

  template <typename Tuple, typename T>
  inline constexpr bool contains_v = contains<Tuple, T>::value;


  /**
   * unique
   */
  template <typename Tuple>
  struct unique;

  template <>
  struct unique<std::tuple<>>
  {
    using type = std::tuple<>;
  };

  template <typename Head, typename... Tail>
  struct unique<std::tuple<Head, Tail...>>
  {
    using type = extend_t<
                   std::tuple<Head>,
                   typename unique<
                     remove_t<std::tuple<Tail...>, Head>
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

  template <typename... Tuples>
  struct flatten<std::tuple<Tuples...>>
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

  template <typename Tuple1>
  struct subtract<Tuple1, std::tuple<>>
  {
    using type = Tuple1;
  };

  template <typename Tuple1, typename Tuple2Head, typename... Tuple2Tail>
  struct subtract<Tuple1, std::tuple<Tuple2Head, Tuple2Tail...>>
  {
    using type = typename subtract<
                                   remove_t<Tuple1, Tuple2Head>,
                                   std::tuple<Tuple2Tail...>
                                  >::type;
  };

  template <typename Tuple1, typename Tuple2>
  using subtract_t = typename subtract<Tuple1, Tuple2>::type;

  /**
   * map
   */
  template <typename Tuple, template <typename> class Metafunction>
  struct map;

  template <template <typename> class Metafunction>
  struct map<std::tuple<>, Metafunction>
  {
    using type = std::tuple<>;
  };

  template <typename... TupleArgs, template <typename> class Metafunction>
  struct map<std::tuple<TupleArgs...>, Metafunction>
  {
    using type = std::tuple< Metafunction<TupleArgs>... >;
  };

  template <typename Tuple, template <typename> class Metafunction>
  using map_t = typename map<Tuple, Metafunction>::type;


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
  struct extract : rebind<T, std::tuple> {};

  template <typename T>
  using extract_t = typename extract<T>::type;
}

#endif
