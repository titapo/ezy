# Result

If you are not familiar with the concept of result type (also known as Either or the proposed `std::expected`)
read about it [here](../discussion/result_rationale.md).

## Factory functions

One illustrative use case of `result` type is factory functions. In the following example `Foo` cannot contain
number greater than `20`. In some cases throwing an exception from the constructor may work, but in a lot of
cases it is not acceptable nor desired.

```cpp

using ErrorMsg = std::string;

class Foo
{
  public:
    static ezy::result<Foo, ErrorMsg> create(int n)
    {
      if (n > 20)
      {
        return ErrorMsg{"Too big!"};
      }

      return Foo(n);
    }

    int get_number() const
    {
      return number;
    }

  private:
    explicit Foo(int n)
      : number(n)
    {}

    const int number{0};
};
```

This class above ensures that any `Foo` instance stores a number below 21 and it cannot be changed after.
The `create` factory function validates the input and returns either a `Foo` or an error message.

```cpp
auto foo_result = Foo::create(3);
auto err_result = Foo::create(25);
```

Both lines returns the same type (`ezy::result<Foo, ErrorMsg>`), but with different content.
`.is_success() -> bool`, `.success() -> Foo`, `.error() -> ErrorMsg` is available, but they not recommended
to be used in general: it would yield a lot of if else branches and complicated control flow. There is a safer
and possibly cleaner interface for them.

### Debug

`ezy::result` has a `visit` function which technically equivalent to `std::visit`:

```cpp
foo_result.visit(
  [](const Foo& foo) { std::cout << "Foo: " << foo.get_number() << "\n"; },
  [](const ErrorMsg& err) { std::cout << "Error: " << err << "\n"; }
);
```

So this will print `Foo: 3`, but calling it on `foo_error` will print `Error: Too big!`.

### Transform

Similarly to `ezy::optional` there is a `.map`:

```cpp
int f(Foo)
map(f)

  Foo --------> f() -------> int

  ErrorMsg ----------------> ErrorMsg

```

So by writing:
```cpp
foo_result.map(&Foo::get_number)
```

we get an `ezy::result<int, ErrorMsg>`:
 * if `foo_result` contained a `Foo` inside, then `get_number` was called on it and the returned int is in
   actually returned
 * if `foo_result` contained and `ErrorMsg`, then nothing will happen the same error string will "remain" in
   the returned `ezy::result`

`.map_error` is `.map`'s the counterpart: it maps the error case, but leaves the success case unchanged:

```cpp
ErrorMsg g(ErrorMsg)
map_error(g)

  Foo ------------------------> Foo

  ErrorMsg ------> g() -------> ErrorMsg

```

```cpp

  auto to_upper = [](const std::string& s)
  {
    return ezy::collect<std::string>(ezy::transform(s, [](auto ch) {return std::toupper(ch); }));
  };

  err_result.map_error(to_upper) // ezy::result<Foo, ErrorMsg>: "TOO BIG!"
```

### Type is not everything

Consider the following example:

```cpp
  foo_result.map(&Foo::get_number).map(ezy::to_string).visit(
      [](const std::string& s) { std::cout << "Foo: " << s << "\n"; },
      [](const ErrorMsg& err) { std::cout << "Error: " << err << "\n"; }
      );
```

Draw a diagram for it. (`ErrorMsg` is a typedef for `std::string` so we use `string` here):

```cpp
  .map(&Foo::get_number)    .    map(ezy::to_string)

                            | 
Foo ------> get_number() -------> to_string -->  string

string --------------------------------------->  string
```

So there is a problem here: visit cannot distinguish the two cases, even if they are different from a logical
perspective. (`.visit([](const std::string& s) { ... });` can work, but the same code would handle both cases)

`.map_or_else(FnSuccess, FnError)` is here to rescue: it takes two functions and applies the first if success
case was active, otherwise applies the second function on the error value. So it calls the proper function
even if the types are the same.

```cpp
  foo_result.map(&Foo::get_number).map(ezy::to_string).map_or_else(
      [](const std::string& s) { std::cout << "from Foo: " << s << "\n"; },
      [](const ErrorMsg& err) { std::cout << "from Error: " << err << "\n"; }
      );
```

> Please note that using `.map_or_else` for side effect and not returning anything is not considered as a nice
> thing, Although there is not much nicer alternative now. Some improvements will happen in the future, so
> this tutorial will be updated too.


## Translate exceptions

Using exceptions and `ezy::result` in a mixed manner can result code that is difficult to reason about. And if
one prefers `ezy::result`, some interoperability required with other code which uses exceptions.

### String to int

Standard has `std::stoi` which stands for *string to int*. Its return value is an `int` and when it fails it
throws an exception. Let's wrap it:

```cpp
ezy::result<int, ErrorMsg> to_int(const std::string& str)
{
  try
  {
    return {std::stoi(str)};
  }
  catch (const std::exception& ex)
  {
    return {ex.what()};
  }
}
```

The logic is quite simple: if an exception was caught, return an error with some reasonable information (The
error type could be anything else), or return the actual success value. From now all methods of `ezy::result`
can be used instead of `try`-`catch` blocks.

### Divide and conquer

Implement a small part of a calculator: divide two numbers!

The parameters are given as strings.

```cpp

ezy::result<std::string, std::string>
divide(const std::string& s1, const std::string& s2)
{
  const auto a = to_int(s1);
  const auto b = to_int(s2);

  // ...
}
```

So `a` and `b` are two `ezy::result<int, ErrorMsg>`, that was the ezy part. But `a / b` cannot be written
here, division is desired for two `ints`, so we have to unwrap the internal value:

```cpp
const auto c = a.and_then(
    [&b](auto x)
    {
       return b.map([&x](auto y) { return x / y; });
    });
```

Now break it apart, from outer:

`.and_then` takes a function which potentially could fail. In our case it will be fail if `b` holds error.

```cpp
result<int, ErrorMsg> f(int)

and_then(f)

  int -------> f() ----> int
                 \
  ErrorMsg -------+----> ErrorMsg
```

The nested lambda invokes `b.map()`, since the result of the division is a simple `int`, so it cannot fail
(just wait for it, it will).

The result can be returned easily by writing: `return c.map(ezy::to_string);`.

> Two nested lambdas with captures and hidden logic?! There may be some simpler solution in the future.

If you have already know `curry`, you can replace the inner lambda with it:

```cpp
static constexpr auto div = ezy::experimental::curry(std::divides<>{});
// So `div(15)(5)` and `15 / 5` are (logically) equivalent

const auto c = a.and_then(
    [&b](auto x)
    {
      return b.map(div(x));
    });
```

So this way we can write half of an expression (partial application) `div(x)` and `b.map()` conditionally
invokes it based on `b`'s state. Personally I find it more readable this version than the lambda.

Our calculator is ready to be used now:

```cpp
divide("120", "4").visit(print_line); // 30
divide("1", "4").visit(print_line); // 0 (integer division)
divide("hundred", "4").visit(print_line); // stoi
divide("120", "four").visit(print_line); // stoi
divide("10", "0").visit(print_line); // ?
```

The last line causes division by zero which is undefined behavior, so we should prevent it somehow.

### Divide or not divide

Here we need a safe division function. It can fail which is represented by the return type.
`div` has to be replaced with the new and safe version.

```cpp
  constexpr auto safe_divides = [](int x, int y) -> ezy::result<int, ErrorMsg>
  {
    if (y == 0)
      return {"division by zero"};
    else
      return {x / y};
  };

static constexpr auto div = ezy::experimental::curry(safe_divides);
```

Since division may fail `b.map(div(x))` will no longer compile, so `.and_then` is required

```cpp
    const auto c = a.and_then(
        [&b](auto x) {
          return b.and_then(div(x));
        });
```

And as a result:

```cpp
  divide("10", "0").visit(print_line); // division by zero
```

Next: [ezy::experimental::nullable](08_nullable.md)
