# Features

In the previous tutorial `ezy::features::printable` was used, but it is not the only feature which makes
strong types more convenient. Arithmetic operations and comparisons are also fundamental features.
In general they are just proxy functions for the underlying type's operation.
These features can be found int `<ezy/features/arithmetic.h>`.

## Adding strong types

```cpp

  using Integer = ezy::strong_type<int, struct IntegerTag,
                                                          ezy::features::printable,
                                                          ezy::features::addable,
                                                          ezy::features::subtractable
                                  >;

    const Integer a{5};
    const Integer b{15};
    const Integer c{10};

    std::cout << a + b - c << "\n";

    Integer d{5};
    d += Integer{3};
    d -= Integer{10};

    std::cout << d << "\n";
```

`addable` allows the `+` and `+=` operators, while `subtractable` allows the `-` and `-=` operators. There is
a composite `ezy::features::additive` which contains both of them.

## Comparing strong types


```cpp
  using Integer = ezy::strong_type<int, struct IntegerTag,
                                                          ezy::features::printable,
                                                          ezy::features::additive,
                                                          ezy::features::equal_comparable
                                  >;

    Integer a{5};
    a += Integer{10};
    std::cout << std::boolalpha << (a == Integer{15}) << "\n";
    std::cout << std::boolalpha << (a != Integer{15}) << "\n";
```

`==` and `!=` are also very useful, alongside with `greater`, `greater_equal`, `less`, `less_equal`, for `>`,
`>=`, `<`, `<=` respectively.

## Multiply

Similarly to `additive`, `ezy::features::multiplicative` is composed by `multiplyable` (`*`, `*=`) and
`divisible` (`/`, `/=`).

## Disclaimer

Those arithmetic features are not battle-tested yet, so if you experience a bug or any inconvenience please
let me know.

Maybe someone wants a `strong_type` be multipliable with a scalar (partially supported), someone else wants
dimension aware operations (like, distance divided by time results speed), or support for affine spaces. Any
of these directions can be pursued, please let me know about your use case.

Next tutorial: [Iterating containers](03_iterable.md)
