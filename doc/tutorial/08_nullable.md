# Nullable

In the previous chapter we met `ezy::optional`: it can make the code more expressive, by explicitly showing
that a value might not be there. In other case there can be a natural bottom value which can represent that a
value is not filled, like `0` or `-1` for numbers, empty for strings and containers, `nullptr` for pointers.
For those types wrapping in an `optional` can cause confusion. While a `nullopt` is obviously indicate the
absence of the value, the bottom value can mean logically the same thing. There can be multiple solutions for
this problem, `ezy` nullable features is one of them.

The basic idea is storing the underlying value (`int` for example), and add an optional-like interface based
on it's bottom value.

> All of these features are under `experimental` namespace: naming is still subject to change. We will see
> that the following features can be used for more than just expressing bottom value.

## Default

Maybe the most common case is using the default constructed element as a non-value. It is `0` for numbers,
empty for strings and containers.

```cpp
using DefaultNullable = ezy::strong_type<int, struct DefaultNullableTag, ezy::features::experimental::default_nullable>;
```

It is a simple strong type for `int`. The `default_nullable` feature just extends its interface by adding
`.has_value`, `.value`, `.value_or` and some other functions.

```cpp
  DefaultNullable{15}.has_value(); // true
  DefaultNullable{}.has_value(); // false
  DefaultNullable{0}.has_value(); // false
```

Default construction gives `0` which is considered as a null value in our case.

```cpp
  DefaultNullable{15}.value() == 15
  DefaultNullable{}.value() == 0
```

`.value()` is an unchecked accessor. In general accessing a value without checking its validity can be
undefined behavior. For now, since DefaultNullable is just an `int` it simply gives back the stored value.

This simple feature can make one's code more expressive, but to be honest it might do not worth too much.
Happily, there are more options available

## Special value

There are some cases when a default constructed element is a valid value and we need a deliberately selected
value to represent absence. For now it will be `-1`.

```cpp
  using NullableAs = ezy::strong_type<int, struct NullableAsTag,
        ezy::features::experimental::basic_nullable_as<ezy::experimental::value_provider<-1>>>;
```

`nullable_as<>` takes a function object which returns a value treated as null, here we are using
`value_provider<-1>`, which is a C++17-compatible way to say `std::integral_constant<int, -1>`. The following
code should be self explaining.

```cpp
  static_assert(NullableAs{15}.has_value());
  static_assert(NullableAs{}.has_value());
  static_assert(!NullableAs{-1}.has_value());

  static_assert(NullableAs{15}.value() == 15);
  static_assert(NullableAs{-1}.value() == -1);
```

There is one possibly thing that could be error prone: namely the convention of the default constructed
element should represent a null value, which this is obviously not true in this case.

To mitigate this burden NullableAs comes with a factory function: `make_null`

```cpp
  static_assert(NullableAs::make_null().has_value() == false);
  static_assert(NullableAs::make_null().value() == -1);
```

Please note that it is possible to write the `-1` only at one place in the code, and it not necessary to
repeat it when using this type. Was it `0` or `-1`?

## Custom comparison

So far, the provided null value was checked for equality, but other comparison operator can be specified as a
second parameter, for example: 'value is considered to be null if it's less than or equal to -1'.

```cpp
  using Nullable = ezy::strong_type<int, struct Tag,
        ezy::features::experimental::nullable_as<
          ezy::experimental::value_provider<-1>,
          std::less_equal<>
        >
      >;

  static_assert(Nullable{10}.has_value());
  static_assert(Nullable{}.has_value());
  static_assert(!Nullable{-1}.has_value());
  static_assert(!Nullable{-10}.has_value());
```

Is there something that prevents to have a comparison which evaluates to false for the provided null value,
like less, greater etc. without equal? Well, not directly, but there is an `is_consistent()` static method for
help.

```cpp
  static_assert(Nullable::is_consistent());
```

It is a good practice to have this validation next to the definition.

> The value provider and the comparison object is recommended to marked `constexpr`, but not forced. This is
> the reason why this static assertion is not automated. (But it can work in a runtime assertion)

## Custom condition

When the underlying type constructor is not free, constructing a value just for comparison would cause
overhead. Furthermore one instance should be enough to decide if it has a value or not. For example: 'is this
string empty?'. Here `nullable_if` will be used:

```cpp
  using NullableStr = ezy::strong_type<std::string, struct Tag,
        ezy::features::experimental::nullable_if<
          decltype(&std::string::empty)
          >
    >;
```

The problem is: this does not compile. If it did, it would crash: its type is a pointer so technically
speaking it has an internal state (where it points to), which is not passed, even if the value could be
managed in compile time as a template parameter. This simple helper exactly does this:

```cpp
  // not in ezy (yet!)
  template <auto Fn>
  struct fn_wrap
  {
    template <typename...Args>
    constexpr bool operator()(Args&&...args) const
    {
      return ezy::invoke(Fn, args...);
    }
  };

  using NullableStr = ezy::strong_type<std::string, struct Tag,
        ezy::features::experimental::nullable_if<
          fn_wrap<&std::string::empty>
          >
    >;
```

> In this special case `decltype(ezy::empty)` (from `<ezy/algorithm.h>`) works as well.

> `nullable_if` has no ability to `make_null`, but if you need it, please let me know your use case. :)

## Handling error codes

There is a common pattern - mostly in C - to return an `int`, where interpretation is the following:
 - if succeeded: `>= 0` (how many bytes written)
 - if failed: `< 0` (error code)

Some other functions:
 -  if succeeded: returns `0`
 - `!= 0` otherwise

`nullable_*` can help to mitigate the cognitive burden of working with functions like these. And they can give
something more...

> The name *nullable* suggests an optional-like behavior, but wrapping these values that can hold *either* a
> return value or an error code reminds me more of a `result` than an `optional`. And that point the name
> *nullable* becomes inaccurate.

## Unwrapper

There is one more parameter for both `nullable_as` and `nullable_if`: it is called *unwrapper*. To understand
it consider a pointer (`T*`) wrapped by a `nullable_as`: the value provider returns `nullptr`, the comparator
is left to equality, and unwrapper is a dereference (`operator*`) call. So `.value()` returns a `T&` now
instead of a pointer.

Ezy has this template: `ezy::pointer<T>`. And it gives something more...

