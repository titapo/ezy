#ifndef STRONG_TYPE_H_INCLUDED
#define STRONG_TYPE_H_INCLUDED

#include <iostream>
#include <type_traits>

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

    strong_type(const strong_type& rhs)
      : _value(rhs.get()) {}

    strong_type& operator=(const strong_type& rhs)
    {
      _value = rhs.get();
      return *this;
    }

    strong_type& operator=(strong_type&& rhs)
    {
      _value = std::move(rhs.get());
      return *this;
    }

    template <typename... Args>
    /*explicit*/ strong_type(Args&&... args
        //, std::enable_if_t<std::is_constructible_v<type, Args...>>* = nullptr
        //, std::enable_if_t<detail::is_braces_constructible<T, Args...>::value>* = nullptr
        //, std::enable_if_t<(sizeof...(Args) != 1) || (!std::is_same_v<std::decay_t<typename detail::headof<Args...>::type>, strong_type>)>* = nullptr
        )
      : _value{std::forward<Args>(args)...}
    {}

    // construction from other
    strong_type(strong_type&& rhs
        , std::enable_if_t<std::is_move_constructible_v<T> || std::is_trivially_move_constructible_v<T>>* = nullptr
        )
      : _value(std::move(rhs).get()) // will it be copied?
    {}

    //strong_type(const strong_type& rhs) = default;

    T& get() & { return _value; }
    decltype(auto) get() &&
    {
      if constexpr (std::is_lvalue_reference_v<T>)
        return _value;
      else
        return std::move(_value);
    }
    const T& get() const & { return _value; }

    explicit operator T() { return _value; }
    explicit operator T() const { return _value; }

    // swap
  private:
    T _value;
};

// TODO if not already a reference?
template <typename T, typename Tag, template<typename> class... Features>
using strong_type_reference = strong_type<typename std::add_lvalue_reference<T>::type, Tag, Features...>;

template <typename T, template<typename> class crtp_type>
struct crtp
{
  using that_type = T;
  T& that() & { return static_cast<T&>(*this); }
  const T& that() const & { return static_cast<const T&>(*this); }
  T&& that() && { return static_cast<T&&>(*this); }
};

template <typename T, template<typename> class crtp_type>
struct strong_crtp : crtp<T, crtp_type>
{
  auto& underlying() { return this->that().get(); }
  const auto& underlying() const { return this->that().get(); }
};

template <typename T, template<typename...> class crtp_type>
struct crtp_with_params
{
  T& that() { return static_cast<T&>(*this); }
  const T& that() const { return static_cast<const T&>(*this); }
};

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
struct get_underlying_type
{
  static_assert(is_strong_type_v<S>, "is not a strong type");
  using type = typename S::type;
};

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

template <typename ST>
struct strip_strong_type
{};

template <typename T, typename Tag, template <typename> class... Features>
struct strip_strong_type<strong_type<T, Tag, Features...>>
{
  using type = strong_type<T, Tag>;
};

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

/*
template <typename ST, typename Feature>
struct has_feature : std::false_type
{
};

template <typename T, typename Tag, template <typename> class... NeedleFeature, template <typename> class... Features>
struct has_feature<strong_type<T, Tag, Features...>, NeedleFeature<strong_type<T, Tag, Features...>>>
{
  static constexpr bool value = std::is_same_v<
    Feature<T, Tag, Features...>, Features<T, Tag, Features...>>...;
  //using type = std::tuple<Features<strong_type<T, Tag, Features...>>...>;
};

template <typename ST, typename NeedleFeature>
using has_feature_t = typename has_feature<ST, NeedleFeature>::type;
*/

/**
 * features
 */
template <typename T>
struct printable : crtp<T, printable>
{
  std::ostream& print_to_stream(std::ostream& ostr) const
  {
    return ostr << this->that().get();
  }
};

template <typename T, typename Tag, template <typename> class... Features>
std::ostream& operator<<(std::ostream& ostr, const strong_type<T, Tag, Features...>& strong)
{
  return strong.print_to_stream(ostr);
}

template <typename T>
struct clonable : crtp<T, clonable>
{
  T clone() const
  {
    return {this->that().get()};
  }
};

template <typename strong_type_t>
struct implicit_convertible;

template <typename underlying_type, typename Tag, template <typename> class... Features>
struct implicit_convertible<strong_type<underlying_type, Tag, Features...>> : crtp<strong_type<underlying_type, Tag, Features...>, implicit_convertible>
{
  operator const underlying_type&() const
  {
    return this->that().get();
  }

  operator underlying_type&()
  {
    return this->that().get();
  }
};

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


template <typename... T>
struct print_type;


#endif
