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
      : value(rhs.get()) {}

    strong_type& operator=(const strong_type& rhs)
    {
      value = rhs.get();
      return *this;
    }

    strong_type& operator=(strong_type&& rhs)
    {
      value = std::move(rhs.get());
      return *this;
    }

    template <typename... Args>
    explicit strong_type(Args&&... args
        //, std::enable_if_t<std::is_constructible_v<type, Args...>>* = nullptr
        //, std::enable_if_t<detail::is_braces_constructible<T, Args...>::value>* = nullptr
        //, std::enable_if_t<(sizeof...(Args) != 1) || (!std::is_same_v<std::decay_t<typename detail::headof<Args...>::type>, strong_type>)>* = nullptr
        )
      : value{std::forward<Args>(args)...}
    {}

    // construction from other
    strong_type(strong_type&& rhs
        , std::enable_if_t<std::is_move_constructible_v<T> || std::is_trivially_move_constructible_v<T>>* = nullptr
        )
      : value(std::move(rhs).get()) // will it be copied?
    {}

    //strong_type(const strong_type& rhs) = default;

    T& get() & { return value; }
    decltype(auto) get() &&
    {
      if constexpr (std::is_lvalue_reference_v<T>)
        return value;
      else
        return std::move(value);
    }
    const T& get() const & { return value; }

    explicit operator T() { return value; }
    explicit operator T() const { return value; }

    // swap
  private:
    T value;
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
struct addable : crtp<T, addable>
{
  T operator+(T const& other) const { return T(this->that().get() + other.get()); }
  T& operator+=(T const& other)
  {
    this->that().get() += other.get();
    return this->that();
  }
};

template <typename T>
struct subtractable : crtp<T, subtractable>
{
  T operator-(T const& other) const { return T(this->that().get() - other.get()); }
  T& operator-=(T const& other)
  {
    this->that().get() -= other.get();
    return this->that();
  }
};

template <typename T>
struct equal_comparable : crtp<T, equal_comparable>
{
  bool operator==(const T& rhs) const
  {
    return this->that().get() == rhs.get();
  }

  bool operator!=(const T& rhs) const
  {
    return !(this->that() == rhs);
  }
};

// nicer solution?
template <typename N>
struct multipliable_by
{
  template <typename T>
  struct internal : crtp<T, internal>
  {
    using numtype = N;
    T operator*(numtype other) const { return T(this->that().get() * other); }
    T& operator*=(numtype other)
    {
      this->that().get() *= other;
      return this->that();
    }
  };
};

template <typename N>
struct divisible_by
{
  template <typename T>
  struct internal : crtp<T, internal>
  {
    using numtype = N;
    T operator/(numtype other) const { return T(this->that().get() / other); }
    T& operator/=(numtype other)
    {
      this->that().get() /= other;
      return this->that();
    }
  };
};

template <typename T>
using divisible_by_int = divisible_by<int>::internal<T>;

template <typename N>
struct multiplicative_by
{
  template <typename T>
  struct internal : multipliable_by<N>::template internal<T>, divisible_by<N>::template internal<T>
  {
    //using numtype = N;
  };
};

template <typename T>
using multiplicative_by_int = multiplicative_by<int>::internal<T>;

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

template <typename T>
struct additive : addable<T>, subtractable<T> {};

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

template <typename T>
struct has_iterator : crtp<T, has_iterator>
{
  auto begin() { return std::begin(this->that().get()); }
  auto begin() const { return std::begin(this->that().get()); }
  auto cbegin() { return std::cbegin(this->that().get()); }
  auto cbegin() const { return std::cbegin(this->that().get()); }

  auto end() { return std::end(this->that().get()); }
  auto end() const { return std::end(this->that().get()); }
  auto cend() { return std::cend(this->that().get()); }
  auto cend() const { return std::cend(this->that().get()); }
};

#include <algorithm>

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

#include "range.h"

/*
template <typename Range, typename Transformation>
struct algo_iterable_range_view;
*/

template <typename T>
struct inherit_range_view_features : crtp<T, inherit_range_view_features>
{
  using const_iterator = typename T::const_iterator;
};

using notag_t = void;

#include <functional>
#include <numeric> // for accumulate and reduce
#include "Enumeration.h"
template <typename T>
struct algo_iterable : crtp<T, algo_iterable>
{
  template < typename UnaryFunction >
  auto for_each(UnaryFunction f)
  {
    return std::for_each(this->that().get().begin(), this->that().get().end(), f);
  }

  template <typename UnaryFunction>
  auto for_each(UnaryFunction f) const
  {
    return std::for_each(this->that().get().begin(), this->that().get().end(), f);
  }

  template <typename UnaryFunction>
  auto map(UnaryFunction&& f) const
  {
    using range_type = typename std::remove_reference<typename T::type>::type;
    using result_range_type = range_view<range_type, UnaryFunction>;
    using algo_iterable_range_view = strong_type<result_range_type, notag_t, has_iterator, algo_iterable>;
    return algo_iterable_range_view(result_range_type(this->that().get(), std::forward<UnaryFunction>(f)));
  }

  template <typename UnaryFunction>
  auto flat_map(UnaryFunction&& f) const
  {
    // FIXME
    using range_type = typename std::remove_reference<typename T::type>::type;
    using result_range_type = range_view<range_type, UnaryFunction>;
    using algo_iterable_range_view = strong_type<result_range_type, notag_t, has_iterator, algo_iterable>;
    return algo_iterable_range_view(result_range_type(this->that().get(), std::forward<UnaryFunction>(f)));
  }

  template <typename RhsRange>
  auto concatenate(const RhsRange& rhs) const
  {
    using range_type = typename std::remove_reference<typename T::type>::type;
    using result_range_type = concatenated_range_view<range_type, RhsRange>;
    using algo_iterable_range_view = strong_type<result_range_type, notag_t, has_iterator, algo_iterable>;
    return algo_iterable_range_view(result_range_type(this->that().get(), rhs));
  }

  template <typename Predicate>
  auto filter(Predicate&& predicate) const
  {
    using range_type = typename std::remove_reference<typename T::type>::type;
    using result_range_type = range_view_filter<range_type, Predicate>;
    using algo_iterable_range_view = strong_type<result_range_type, notag_t, has_iterator, algo_iterable>;
    return algo_iterable_range_view(result_range_type(this->that().get(), std::forward<Predicate>(predicate)));
  }

  template <typename Predicate>
  auto find(Predicate&& predicate) const
  {
    using range_type = typename std::remove_reference<typename T::type>::type;
    using result_type = Optional<typename range_type::value_type>;
    const auto found = std::find_if(this->that().get().begin(), this->that().get().end(), std::forward<Predicate>(predicate));
    if (found != this->that().get().end())
      return result_type(*found);
    else
      return result_type();
  }

  template <typename Element> // TODO contained element should be accepted (or at least comparable)
  bool contains(Element&& needle) const
  {
    return find([&](const auto& element) { return element == needle; }).has_value();
  }

  template <typename Type>
  Type accumulate(Type init) 
  {
    return std::accumulate(this->that().get().begin(), this->that().get().end(), init);
  }

  template <typename Type, typename BinaryOp>
  Type accumulate(Type init, BinaryOp op) 
  {
    return std::accumulate(this->that().get().begin(), this->that().get().end(), init, op);
  }

  /*
   * not found in gcc even if numeric has been included
  template <typename Type>
  Type reduce(Type init) 
  {
    return std::reduce(this->that().get().begin(), this->that().get().end(), init);
  }
  */

  template <typename Predicate>
  auto partition(Predicate&& predicate) const
  {
    using range_type = typename std::remove_reference<typename T::type>::type;
    using concrete_predicate_type = std::function<bool(const typename range_type::value_type&)>;
    using result_range_type = range_view_filter<range_type, concrete_predicate_type>;
    using algo_iterable_range_view = strong_type<result_range_type, notag_t, has_iterator, algo_iterable>;

    const auto negate_result = [](const Predicate& original_predicate)
    {
      return [=](const typename range_type::value_type& v)
      { return !original_predicate(v); };
    };

    return std::make_tuple(
        algo_iterable_range_view(result_range_type(this->that().get(), predicate)),
        algo_iterable_range_view(result_range_type(this->that().get(), negate_result(predicate)))
        );
  }

  auto slice(const unsigned from, const unsigned until) const
  {
    using range_type = typename std::remove_reference<typename T::type>::type;
    using result_range_type = range_view_slice<range_type>;
    using algo_iterable_range_view = strong_type<result_range_type, notag_t, has_iterator, algo_iterable>;
    return algo_iterable_range_view(result_range_type(this->that().get(), from, until));
  }

  template <typename Predicate>
  bool all(Predicate&& predicate) const
  {
    return std::all_of(this->that().get().begin(), this->that().get().end(), std::forward<Predicate>(predicate));
  }

  template <typename Predicate>
  bool any(Predicate&& predicate) const
  {
    return std::any_of(this->that().get().begin(), this->that().get().end(), std::forward<Predicate>(predicate));
  }

  template <typename Predicate>
  bool none(Predicate&& predicate) const
  {
    return std::none_of(this->that().get().begin(), this->that().get().end(), std::forward<Predicate>(predicate));
  }

  /*
  auto grouped(const unsigned group_size) const
  {
    using range_type = typename std::remove_reference<typename T::type>::type;
    using group_range_type = range_view_slice<range_type>;
    using algo_iterable_group_type = strong_type<group_range_type, notag_t, has_iterator, algo_iterable>;
    using wrapper_range = basic_range_view<algo_iterable_group_type>;
    using algo_iterable_wrapper_range = strong_type<wrapper_range, notag_t, has_iterator, algo_iterable>;
    //using algo_iterable_range_view
    return algo_iterable_wrapper_range(algo_iterable_group_type(this->that().get(), 0, group_size));
  }
  */

  template <typename OtherRange>
  auto zip(const OtherRange& other_range)
  {
    using lhs_range_type = typename std::remove_reference<typename T::type>::type;
    using rhs_range_type = typename std::remove_reference<OtherRange>::type;
    using result_range_type = zip_range_view<lhs_range_type, rhs_range_type>;
    using algo_iterable_range_view = strong_type<result_range_type, notag_t, has_iterator, algo_iterable>;
    return algo_iterable_range_view(result_range_type(this->that().get(), other_range));
  }

  //template <typename OtherRange> // TODO constrain to be a raised (flattenable) range
  auto flatten() const
  {
    using ThisRange = typename std::remove_reference<typename T::type>::type;
    using result_range_type = flattened_range_view<ThisRange> ;
    using algo_iterable_range_view = strong_type<result_range_type, notag_t, has_iterator, algo_iterable>;
    return algo_iterable_range_view(result_range_type(this->that().get()));
  }

  template <typename ResultContainer>
  ResultContainer to() const
  {
    return ResultContainer(this->that().get().begin(), this->that().get().end());
  }

  template <typename ResultContainer>
  auto to_iterable() const
  {
    using algo_iterable_container = strong_type<ResultContainer, notag_t, has_iterator, algo_iterable>;
    return algo_iterable_container(this->that().get().begin(), this->that().get().end());
  }

};

template <typename T>
struct iterable : has_iterator<T>, algo_iterable<T> {};


template <typename... T>
struct print_type;


template <typename T>
struct visitable : crtp<T, visitable>
{
  template <typename... Visitors>
  decltype(auto) visit(Visitors&&... visitors) &
  {
    return std::visit(ezy::overloaded{std::forward<Visitors>(visitors)...}, this->that().get());
  }

  template <typename... Visitors>
  decltype(auto) visit(Visitors&&... visitors) const &
  {
    return std::visit(ezy::overloaded{std::forward<Visitors>(visitors)...}, this->that().get());
  }

  // TODO check it
  template <typename... Visitors>
  decltype(auto) visit(Visitors&&... visitors) &&
  {
    //print_type<decltype((*this).that().get()), decltype(std::move(*this).that().get())>{};
    return std::visit(ezy::overloaded{std::forward<Visitors>(visitors)...}, std::move(*this).that().get());
  }
};

// TODO support for std::get

template <typename Subject, typename Pattern>
struct forward_const
{
  using type = std::conditional_t<std::is_const_v<std::remove_reference_t<Pattern>>, std::add_const_t<Subject>, Subject>;
};

template <typename Subject, typename Pattern>
using forward_const_t = typename forward_const<Subject, Pattern>::type;

template <typename Subject, typename Pattern>
struct forward_reference
{
  using type = std::conditional_t<
    std::is_reference_v<Pattern>,
    std::conditional_t<
      std::is_lvalue_reference_v<Pattern>, std::add_lvalue_reference_t<Subject>, std::add_rvalue_reference_t<Subject>
    >,
    Subject
  >;
};

template <typename Subject, typename Pattern>
using forward_reference_t = typename forward_reference<Subject, Pattern>::type;

template <typename Subject, typename Pattern>
struct forward_c_ref
{
  using type = forward_reference_t<forward_const_t<Subject, Pattern>, Pattern>;
};

template <typename Subject, typename Pattern>
using forward_c_ref_t = typename forward_c_ref<Subject, Pattern>::type;


template <typename T>
struct result_like_continuation : crtp<T, result_like_continuation>
{
  using base = crtp<T, result_like_continuation>;
  using self_type = result_like_continuation;

  template <typename Type>
  struct result_trait;

  template <template <typename...> class Wrapper, typename Success, typename Error>
  struct result_trait<Wrapper<Success, Error>>
  {
    using type = Wrapper<Success, Error>;
    using success_type = std::decay_t<decltype(std::get<0>(std::declval<type>()))>;
    using error_type = std::decay_t<decltype(std::get<1>(std::declval<type>()))>;

    template <typename NewSuccess>
    struct rebind_success
    {
      using type = Wrapper<NewSuccess, error_type>;
    };

    template <typename NewSuccess>
    using rebind_success_t = typename rebind_success<NewSuccess>::type;
  };


  template <typename ST, typename Fn>
  static constexpr decltype(auto) map_impl(ST&& t, Fn&& fn)
  {
    using dST = std::decay_t<ST>;
    using trait = result_trait<typename dST::type>;

    // TODO static_assert(std::is_invocable_v<Fn(success_type)>, "Fn must be invocable with success_type");
    //TODO using map_result_type = std::invoke_result_t<Fn(success_type)>;
    using fn_result_type = decltype(fn(std::declval<typename trait::success_type>()));

    using R = typename trait::template rebind_success_t<fn_result_type>;
    using return_type = rebind_strong_type_t<dST, R>;

    /* TODO find out which one is better?
    if (std::holds_alternative<success_type>(this->that().get()))
      return return_type(std::invoke(std::forward<Fn>(fn), std::get<success_type>(this->that().get())));
    else
      return return_type(std::get<error_type>(this->that().get()));
      */

    return return_type(std::visit(ezy::overloaded{
        [&](forward_c_ref_t<typename trait::success_type, ST> s) { return R{std::invoke(std::forward<Fn>(fn), std::forward<decltype(s)>(s))}; },
        [](forward_c_ref_t<typename trait::error_type, ST> e) { return R{std::forward<decltype(e)>(e)}; }
        }, std::forward<ST>(t).get()));
  }

  template <typename ST, typename Fn>
  static constexpr decltype(auto) and_then_impl(ST&& t, Fn&& fn)
  {
    using dST = std::decay_t<ST>;
    using trait = result_trait<typename dST::type>;

    using fn_result_type = decltype(fn(std::declval<typename trait::success_type>()));

    using R = fn_result_type;
    using new_underlying_type = plain_type_t<R>; // still not totally OK -> underlying type is also a strong type?
    using return_type = rebind_strong_type_t<dST, new_underlying_type>;

    using new_trait = result_trait<typename return_type::type>;
    static_assert(std::is_same_v<typename new_trait::error_type, typename trait::error_type>, "error types must be the same");

    return return_type(std::visit(ezy::overloaded{
        [&](forward_c_ref_t<typename trait::success_type, ST> s) -> R { return std::invoke(std::forward<Fn>(fn), std::forward<decltype(s)>(s)); },
        [](forward_c_ref_t<typename trait::error_type, ST> e) { return R{std::forward<decltype(e)>(e)}; }
        }, std::forward<ST>(t).get()));
  }


  template <typename Fn>
  constexpr decltype(auto) map(Fn&& fn) &
  {
    return map_impl((*this).that(), std::forward<Fn>(fn));
  }

  template <typename Fn>
  constexpr decltype(auto) map(Fn&& fn) const &
  {
    return map_impl((*this).that(), std::forward<Fn>(fn));
  }

  template <typename Fn>
  constexpr decltype(auto) map(Fn&& fn) &&
  {
    return map_impl(std::move(*this).that(), std::forward<Fn>(fn));
  }

  template <typename Fn>
  constexpr decltype(auto) and_then(Fn&& fn) &
  {
    return and_then_impl((*this).that(), std::forward<Fn>(fn));
  }

  template <typename Fn>
  constexpr decltype(auto) and_then(Fn&& fn) const&
  {
    return and_then_impl((*this).that(), std::forward<Fn>(fn));
  }

  template <typename Fn>
  constexpr decltype(auto) and_then(Fn&& fn) &&
  {
    return and_then_impl(std::move(*this).that(), std::forward<Fn>(fn));
  }
};


#endif
