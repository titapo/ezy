# Optional

`std::optional` is a really useful type template to express if something is there or not. However in practice,
its interface is not always so comfortable. `ezy` mitigates this issue by defining a feature similarly to
`iterable`, and additionally wraps `std::optional` to extending its interface: meet `ezy::optional`

```cpp
// pseudocode from ezy
namespace ezy
{
  template <typename T>
  using optional = extended_type<std::optional<T>, /*...*/>;
}
```

The main point here that `std::optional` can be replaced by `ezy::optional` in most cases and where it doesn't
`.get()` still returns an `std::optional`.

Well, then what is the point here?

## Up to the limits

```cpp
ezy::optional<int> even(int i)
{
  if (i % 2 == 0)
    return i;
  else
    return std::nullopt;
}
```

Here we have a function which returns an even number or a `nullopt`.
(`ezy::optional` is an `extended_type` so it does not require explicit constructor.)
So far there is no difference compared with `std::optional`.

```cpp
std::cout << even(10).value() << "\n"; // prints 10
std::cout << even(11).value() << "\n"; // oops! throws std::bad_optional_access
```

`.value()` is the unsafe accessor and must not be called, unless it was verified that optional actually holds
a value:

```cpp
const auto from_eleven = even(11);
if (from_eleven.has_value())
{
  std::cout << from_eleven.value() << "\n";
}
```

So far we could have done it with `std::optional` as well. But we have reached its limits and we cannot do any better.

## Beyond the limits

For sake of simplicity:

```cpp
auto print_line = [](cons auto& i) {
  std::cout << i << "\n";
};
```

Calling a function based on optional's state shouldn't be that hard:

```cpp
  // neither of those works
  //from_eleven.inspect(print_line);
  //from_eleven.tee(print_line);
```

So `print_line` will be called with `10` if optional contains the value `10`, otherwise the function will not
be called at all.

```
void f(int)
inspect/tee(f)

            f()
           /
  int ----+---------> int

  nullopt ----------> nullopt
```

(When any of those will be available, this tutorial will be updated. Until then `value_or` will be used in the
rest of this tutorial.)

```cpp
print_line(even(10).value_or(-100)); // 10
print_line(even(11).value_or(-100)); // -100
```

`value_or` returns the stored value, or an alternative in case of `nullopt`. (Standard supports it too.)

```
value_or

  int --------+-------> int
             /
  nullopt --+
```

## Mapping

`inspect`/`tee` is for doing some side effect, which generally does not return anything. Sometimes it is
useful to return something

```cpp
  print_line(even(10).map([](int i) {return i + 10;}).value_or(-100)); // 20
  print_line(even(11).map([](int i) {return i + 10;}).value_or(-100)); // -100
```

```
int f(int)
map(f)

  int ------> f() ----> int

  nullopt ------------> nullopt
```

Map calls a function and return an optional containing its result. In case of `nullopt`, nothing happens.

## Mapping over types

`map` handles functions which return a different type that its parameter, so here the return type will be
`ezy::optional<string>` instead `ezy::optional<int>`.

```cpp
string f(int)
map(f)

  int ------> f() ------> string

  nullopt --------------> nullopt
```

```cpp
  print_line(even(10).map(ezy::to_string).value_or("none")); // 10
  print_line(even(11).map(ezy::to_string).value_or("none")); // none
```

`map` and `value_or` can be combined into a `map_or`

```cpp
  print_line(even(10).map_or(ezy::to_string, "none")); // 10
  print_line(even(11).map_or(ezy::to_string, "none")); // none
```

## And then?

There are other kind of functions which themselves takes a parameter and returns an optional (eg. `even`).
Sometimes they are not at the beginning, but in the middle of the pipeline.

```cpp
optional<T> f(int)

and_then(h)

  int ------> h() ----> T
                \
  nullopt -------+----> nullopt
```
```cpp
  auto half = [](int i){ return i / 2; };

  print_line(from_ten.map(half).and_then(even).map_or(ezy::to_string, "none")); // none
  print_line(from_eleven.map(half).and_then(even).map_or(ezy::to_string, "none")); // none
  print_line(even(16).map(half).and_then(even).map_or(ezy::to_string, "none")); // 8
```

Full visualization:
```
          even(16)   .  map(half)    . and_then(even) . map_or(ezy::to_string, "none");

      |              |               |                |                                |

int -----> even() --------> half() -----> even() ---------> to_string ---------+---------> string
                \                              \                              /
        nullopt  +------------------------------+----------------------------+
```

## Limitations and refinements

`std::optional` does not support references, so `ezy::optional` does not support it either. `ezy::pointer` can
be used as a replacement for optional references, since it also has this advanced interface.

Sometimes there is a natural bottom value for a type, which can be considered null/invalid. Putting those
types into an `optional` would cause redundancy, not mentioning the space overhead. There are features
(currently called `nullable`) to simply extend their interface with these functions. (They are marked
`experimental`, because naming is not finalized yet.)

