#ifndef ENUMERATION_H_INCLUDED
#define ENUMERATION_H_INCLUDED

#include <variant>
#include <type_traits>
#include <functional>

namespace ezy
{
  template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
  template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

  struct tuple_overload
  {
    template <typename... Ts>
    auto operator()(Ts... ts)
    { return overloaded{std::forward<Ts>(ts)...}; }
  };
}

template <typename Fn>
struct argument_type;

template <typename R, typename Arg>
struct argument_type<std::function<R(Arg)>>
{
  using type = Arg;
};

template <typename R, typename Arg>
struct argument_type<std::function<R (*)(Arg)>>
{
  using type = Arg;
};


template <typename R, typename C, typename Arg>
struct argument_type<R (C::*)(Arg)>
{
  using type = Arg;
};

template <typename R, typename C, typename Arg>
struct argument_type<R (C::*)(Arg) const>
{
  using type = Arg;
};

template <typename R, typename C, typename Arg>
struct argument_type<std::function<R (C::*)(Arg)>>
{
  using type = Arg;
};

template <typename R, typename C, typename Arg>
struct argument_type<std::function<R (C::*)(Arg) const>>
{
  using type = Arg;
};

template <typename Fn>
using argument_type_t = typename argument_type<Fn>::type;

// from is_one_of: https://stackoverflow.com/questions/34111060/c-check-if-the-template-type-is-one-of-the-variadic-template-types
template <typename...>
struct is_one_of
{
  static constexpr bool value = false;
};

template <typename T, typename Head, typename... Tail>
struct is_one_of<T, Head, Tail...>
{
  static constexpr bool value = std::is_same_v<T, Head> || is_one_of<T, Tail...>::value;
};

namespace ezy
{
  inline namespace match_cases
  {
    template <typename ResultType, typename CommonType, typename... Actions>
    struct case_option
    {

      template <typename CaseType, typename ActionType
        //, typename = std::enable_if_t<std::is_invocable<ActionType, CaseType>::value>
        //, typename = std::enable_if_t<!(std::is_invocable<ActionType, Actions>::value || ...)> // FIXME
               //typename = std::enable_if_t<std::is_same_v<std::decay_t<argument_type_t<ActionType>>, ResultType(CaseType) > >
               >
      [[nodiscard]] auto on(ActionType&& action) &&
      {
        static_assert(std::is_invocable<ActionType, CaseType>::value, "This method cannot be called for this case!");
        static_assert(!(std::is_invocable<Actions, CaseType>::value || ...), "This case is handled in a previous case!");
        //static_assert(std::is_same<std::result_of_t<ActionType(const CaseType&)>, CommonType>::value, "Different Result types are not allowed!");
        static_assert(std::is_convertible<std::result_of_t<ActionType(const CaseType&)>, CommonType>::value, "incompatible result types!");
        return case_option<ResultType, CommonType, Actions..., ActionType>{std::cref(result), std::tuple_cat(actions, std::tuple(action))};
      }

      template <typename ActionType>
      auto otherwise(ActionType&& action) &&
      {
        return case_option<ResultType, CommonType, Actions..., ActionType>{std::cref(result), std::tuple_cat(actions, std::tuple(action))}();
      }

      template <typename Value>
      auto otherwiseReturn(Value&& value) &&
      {
        return std::move(*this).otherwise([&value](const auto&) {return value;});
      }

      auto otherwiseIgnore() &&
      {
        return std::move(*this).otherwise([](const auto&){});
      }

      CommonType operator()() && {
        return result.match(std::apply(tuple_overload(), actions));
      }

      const ResultType& result;
      std::tuple<Actions...> actions;
    };


    template <typename ResultType, typename CommonType, typename... Actions>
    case_option(ResultType, CommonType, Actions...) -> case_option<ResultType, CommonType, Actions...>;

    template <typename ResultType, typename CommonType>
    case_option(ResultType, CommonType) -> case_option<ResultType, CommonType>;

    template <typename CaseType, typename ResultType, typename Action>
    auto make_case_option(const ResultType& r, Action&& action)
    {
      using CommonType = std::result_of_t<Action(const CaseType&)>;
      return case_option<ResultType, CommonType, Action>{r, std::forward<Action>(action)};
    }
  }
}

#include <optional>
template <typename ResultType, typename ActionType>
struct imcase
{
  const ResultType& r;
  std::optional<ActionType> action;
};

template <typename... Variants>
class Enumeration
{
  public:
    using variant_type = std::variant<Variants...>;

    Enumeration(const Enumeration&) = default;
    Enumeration(Enumeration&&) = default;

    template <typename Type,
             typename = std::enable_if_t<!std::is_same_v<std::decay_t<Type>, Enumeration>>,
             typename = std::enable_if_t<(sizeof...(Variants) > 0)>,
             typename = std::enable_if_t<is_one_of<std::decay_t<Type>, Variants...>::value > // TODO or convertible
             //typename = std::enable_if_t<std::is_constructible_v<variant_type, Type> >
             >
    Enumeration(Type&& t)
      : v(std::forward<Type>(t))
    {}

    Enumeration& operator=(const Enumeration& rhs)
    {
      v = rhs.get();
      return *this;
    }

    const variant_type& get() const
    { return v; }

    template <typename DispatcherType>
    auto match(DispatcherType dispatcher) const
    {
      return std::visit(dispatcher, v);
    }

    template <typename... Actions>
    auto match(Actions&&... actions) const
    {
      return match(std::apply(ezy::tuple_overload(), std::forward_as_tuple(actions...)));
    }

    template <typename DispatcherType>
    auto match(DispatcherType dispatcher)
    {
      return std::visit(dispatcher, v);
    }

    template <typename... Actions>
    auto match(Actions&&... actions)
    {
      return match(std::apply(ezy::tuple_overload(), std::forward_as_tuple(actions...)));
    }
    
    template <typename CaseType, typename ActionType>
    [[nodiscard]] auto on(ActionType&& action)
    {
      return ezy::make_case_option<CaseType>(*this, action);
    }

    template <typename CaseType, typename ActionType>
    [[nodiscard]] auto on(ActionType&& action) const
    {
      return ezy::make_case_option<CaseType>(*this, action);
    }

    template <typename CaseType, typename ActionType>
    [[nodiscard]] auto on2(ActionType&& action)
    {
      if (is<CaseType>())
        return imcase{*this, action};
      else
        return imcase{*this, {}};
    }

    template <typename Type>
    bool is() const
    {
      return std::holds_alternative<Type>(get());
    }

  private:
    std::variant<Variants...> v;
};

using NoneType = std::nullopt_t;

template <typename T>
class Optional : public Enumeration<T, NoneType>
{
  public:
    using Base = Enumeration<T, NoneType>;

    using Base::Enumeration;
    using Base::operator=;

    Optional()
      : Base::Enumeration(std::nullopt)
    {}

    bool has_value() const
    {
      return Base::template is<T>();
    }

    T value_or(const T& alternative) const
    {
      return Base::template on<T>([](const T& value) { return value; })
        .otherwiseReturn(alternative);
    }

    template <typename ActionType>
    [[nodiscard]] auto on_value(ActionType&& action)
    {
      return Base::template on<T>(std::forward<ActionType>(action));
    }

    template <typename F, typename R = Optional<typename std::result_of<F(const T&)>::type> >
    auto map(F&& function) -> R
    {
      return Base::
         template on<T>([&function](const T& t) -> R { return {function(t)} ;})
        .template on<NoneType>([](const NoneType&) -> R { return {std::nullopt}; })
        ();
    }

    template <typename F, typename R = typename std::result_of<F(const T&)>::type>
    auto map_or(F&& function, const R& alternative) const
    {
      return Base::
         template on<T>([&function](const T& t) -> R { return {function(t)} ;})
        .otherwiseReturn(alternative);
    }

    struct const_iterator
    {
      explicit const_iterator(const T* value_ptr)
        : ptr(value_ptr)
      {}
      
      const_iterator& operator++()
      {
        ptr = nullptr;
        return *this;
      }

      bool operator==(const const_iterator& rhs) const
      {
        return ptr == rhs.ptr;
      }

      bool operator!=(const const_iterator& rhs) const
      {
        return !(*this == rhs);
      }

      const T& operator*() const
      {
        return *ptr;
      }

      const T* ptr;
    };

    const_iterator begin() const
    {
      if (has_value())
        return const_iterator{&std::get<T>(Base::get())};

      return end();
    }

    const_iterator end() const
    {
      return const_iterator(nullptr);
    }
};

template <typename T>
auto Some(T&& t) -> Optional<std::decay_t<T>>
{
  return {std::forward<T>(t)};
}


template <typename S, typename E>
class [[nodiscard]] Result : public Enumeration<S, E>
{
  public:
    using Base = Enumeration<S, E>;
    using Success = S;
    using Error = E;

    using Base::Enumeration;

    bool is_succeeded() const
    {
      return Base::template is<Success>();
    }

    bool is_failed() const
    {
      return Base::template is<Error>();
    }

    Optional<Success> success()
    {
      using R = Optional<Success>;
      return Base::template on<Success>([](const Success& value) { return R{value}; })
        .otherwiseReturn(R{std::nullopt});
    }

    template <typename F, typename R = Result<typename std::result_of<F(const Success&)>::type, Error> >
    auto map(F&& function) -> R
    {
      return Base::
         template on<Success>([&function](const Success& s) -> R { return {function(s)} ;})
        .template on<Error>([](const Error& err) -> R { return {err}; })
        ();
    }

    template <typename F, typename R = Result<std::remove_reference_t<typename std::result_of<F(const Success&)>::type>, Error> >
    auto map_or(F&& function, const R& alternative) const -> R
    {
      return Base::
         template on<Success>([&function](const Success& s) -> R { return {function(s)} ;})
        .otherwiseReturn(alternative);
    }

    Success success_or(const Success& alternative) const
    {
      return Base::template on<Success>([](const Success& value) { return value; })
        .otherwiseReturn(alternative);
    }

    template <typename F, typename R = typename std::result_of<F(const Success&)>::type>
    auto and_then(F&& function) -> R
    {
      return Base::
        template on<Success>([&function](const Success& s) -> R { return function(s); })
        .template on<Error>([](const Error& err) -> R { return {err}; })
        ();
    }

    template <typename F, typename R = typename std::result_of<F(const Success&)>::type>
    auto and_then(F&& function) const -> R
    {
      return Base::
        template on<Success>([&function](const Success& s) -> R { return function(s); })
        .template on<Error>([](const Error& err) -> R { return {err}; })
        ();
    }

};


#endif
