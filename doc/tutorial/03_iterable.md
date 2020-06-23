# Iterating containers

As was presented in the previous tutorial `ezy::features::printable` allows the programmer to expose a
behaviour of the underlying type. `ezy`'s fundamental idea about features that some of them expands the
capabilities based on existing ones. For example if there a class with `begin()` and `end()`, stl algorithms
can be used with them. `ezy::features::iterable` provides a convenient interface for them.

Suppose that we want to work with the results of a written test in a school. Our data can be represented the
following way:

```cpp
#include <ezy/strong_type.h>
#include <ezy/features/iterable.h>
#include <vector>
#include <string>

// ...

using Names = ezy::strong_type<std::vector<std::string>, struct NamesTag, ezy::features::iterable>;
using Scores = ezy::strong_type<std::vector<int>, struct PointsTag, ezy::features::iterable>;

Names names{"Alice", "Bob", "Cecil", "David"};
Scores scores{10, 35, 23, 29};
```

`Names` and `Scores` are two strong containers, and two object is created: the first contains the names, the
second one stores the scores they got for the test respectively.

## Print the names

```cpp
names.for_each([](const std::string& name) { std::cout << name << "\n"; });
```

`for_each` is a wrapper for `std::for_each`, so this is roughly equivalent to:

```cpp
std::for_each(names.begin(), names.end(), [](const std::string& name) { std::cout << name << "\n"; });
```

or with a simple for-loop:

```cpp
for (const std::string& name : names)
{
  std::cout << name << "\n";
}
```

Maybe the latter one seems to be more familiar, until we don't make a little abstraction:

```cpp
auto print_line = [](const std::string& name) { std::cout << name << "\n"; };

names.for_each(print_line); // (1) with ezy::features::iterable

std::for_each(names.begin(), names.end(), print_line); // (2)

for (const std::string& name : names)
{
  print_line(name);
} // (3)
```

If you were still not convinced, it's OK. However we will continue with the first option anyway :)

```cpp
print_line("Names:");
names.for_each(print_line);
```

Output:
```
Names:
Alice
Bob
Cecil
David
```

## Prettier output

We should have a nicer output where names are look like as a list:

```cpp
const std::string prefix = " - ";
auto prepend = [&](const std::string& s) { return prefix + s; };

names.map(prepend).for_each(print_line);
```

`prepend` is a simple helper lambda which will be used for mapping the values. And what does `.map` do here?
it calls prepend to each element of `names` and returns a range of the results. `std::transform` would do the
same with the difference that `.map()` provides lazy evaluation.

```
names
[                [
  "Alice"    ->    " - Alice"
  "Bob"      ->    " - Bob"
  "Cecil"    ->    " - Cecil"
  "David"    ->    " - David"
]                ]
```

This still can be expressed as a for-loop:

```cpp
for (const std::string& name : names)
{
  print_line(prepend(name));
}
```

So the output is:
```
names:
 - Alice
 - Bob
 - Cecil
 - David
```

## Number of passed tests

Let assume that a test is passed if has at least 25 score.

```cpp
bool test_passed(int score)
{ return score >= 25; }

std::cout << "Number of passed tests: " << scores.filter(test_passed).size() << "\n";
```

Filter returns a range with only the elements that passed the predicate and `size` will return the number of
its elements. It is important to note that `scores` doesn't changed, filter gives a lazy view of it.

## Print the names and the scores together

After we have some intuition how can work with those algorithms, we should make some meaningful action: show
the names alongside with the scores they get. There is a little helper to format this:

```cpp
const auto display = [](const std::string name, int score) {
  return name + " (" + ezy::to_string(score) + ")";
};
```

```cpp
  names
       .zip_with(display, scores)
       .map(prepend)
       .for_each(print_line);
```

`zip_with` is very similar to `map`: it calls `display` with every element of `names`, and additionally takes
elements from `scores`. (Its STL counterpart is `std::transform` invocation on two ranges)

```
names            scores
[                [                                         [
  "Alice"          10           display("Alice", 10)  ->     "Alice (10)"
  "Bob"            35           display("Bob", 35)    ->     "Bob (35)"
  "Cecil"          23           display("Cecil", 23)  ->     "Cecil (23)"
  "David"          29           display("David", 29)  ->     "David (29)"
]                ]                                         ]
```

And since `display` returns a `string`, the following `map(prepend)` and `for_each(print_line)` can be used without any change.

```
 - Alice (10)
 - Bob (35)
 - Cecil (23)
 - David (29)
```

## Print who passed

So far we could use `zip_with(display, ...)`, it zipped the data into one string which is human readable, but
after that we cannot filter for scores. One could try zipping names and filtered scores, like: 

```cpp
// wrong
names
     .zip_with(display, scores.filter(test_passed))
     .map(prepend)
     .for_each(print_line);

/* prints:
 - Alice (35)
 - Bob (23)
*/
```

The result is simply wrong, because the first two names is used, however Bob and David has passed the test.

So first we have to zip the corresponding names and scores and then filter. To achieve this the basic `.zip()` function can be used, which turns every element into a tuple, 

```
names            scores
[                [                   [
  "Alice"          10         ->       ("Alice", 10)
  "Bob"            35         ->       ("Bob", 35)
  "Cecil"          23         ->       ("Cecil", 23)
  "David"          29         ->       ("David", 29)
]                ]                   ]
```

Logically we try to do something like:
```cpp
// pseudocode
names
     .zip(scores)
     .filter(test_passed)
     .map(display)
     // ...
```

#### Filtering

The problem here is we have a tuple of name and score and `test_passed` cannot be invoked with it. So we need
to manually roll out a lambda, which selects the score from the tuple, and calls `test_passed` on it.

```cpp
.filter([](const auto& name_with_score) {
    return test_passed(std::get<1>(name_with_score));
})
```

> If you are not entirely happy with this solution, then we are on the same side. There is one experimental
component which could possibly help here: `ezy::experimental::compose` it can chain functions:
> `compose(f, g, h)(x)` is logically equivalent to `h(g(f(x)))`
> With this filter can be rewritten `.filter(ezy::experimental::compose(ezy::pick_second, test_passed))`
> (`<ezy/experimental/compose.h>` has to be included)
> Do you think is it better?

#### Mapping

Unfurtunately not only filter is affected, but we cannot call `display` with a tuple, because it expects two
separate parameters, so we also need a helper lambda for it:

```cpp

const auto display_with_name_and_score = [](const auto& name_with_score) {
  return std::apply(display, name_with_score);
};
```

//*
It is still not not desirable. As soon as ezy will have a convenient solution for expressing it, this tutorial
will be updated too.
*/

#### Put all together

```cpp
return names
  .zip(scores)
  .filter([](const auto& name_with_score) {
      return test_passed(std::get<1>(name_with_score));
      })
  .map(display_with_name_and_score);
  .map(prepend)
  .for_each(print_line);
```

