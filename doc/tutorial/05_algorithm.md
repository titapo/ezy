# Algorithms

Take the the functions from the [previous tutorial](03_extended_type.md), where we applied
`ezy::make_extended_reference` to a `std::vector`. There is another way to do so: algorithms.

```cpp
#include <ezy/algorithm.h>

std::vector<std::string> get_words_longer_than(const std::vector<std::string>& words, size_t treshold)
{
  const auto too_long = [treshold](const auto& word) { return word.size() > treshold; };
  return ezy::collect<std::vector<std::string>>(
      ezy::filter(words, too_long)
    );
}
```

Examine inner first: `ezy::filter(words, too_long)` would be equivalent to `words.filter(too_long)` if it was
a strong/extended type, and the `collect<>()` does the same as `.to<>()` would do.

Algorithms used with this syntax is very straightforward if there is only one or two calls, but it can result
in hard-to-follow code for more.

## Printing

Let's print the long words

```cpp

const std::vector<std::string> words{"a", "bb", "looong", "looooong"};
const auto long_words = get_words_longer_than(words, 3);
```

Based on the previous tutorials one could use the `long_words.for_each(print_line)` solution, which actually
works with algorithms as well:

```cpp
ezy::for_each(long_words, print_line);
```

But now the words should be printed the same line, separated by comma.

```cpp
std::cout << ezy::join(long_words, ", ") << "\n";
```
output:
```
looong, looooong
```

## Injection

Add a new word to the list:

```cpp
  const std::vector<std::string> words{"a", "bb", "looong", "looooong", "with, comma, and, space"};
```

Output:
```
looong, looooong, with, comma, and, space
```

To fix this words should be quoted:

```cpp
  auto quote = [](const std::string &s) { return std::string() + "\"" + s + "\""; };
  std::cout << ezy::join(ezy::transform(long_words, quote), ", ") << "\n";
```

Output:
```
"looong", "looooong", "with, comma, and, space"
```

`ezy::transform` is logically the same as `.map()` for `ezy::features::iterable`.

~~[Why doeas iterable feature and algorithms use different names?](discussion)~~

## Enumerate the results

`ezy::enumerate` is a really helpful function: it returns all the elements from a range, but zips them with
their indices. One programmer would fall back the good old for loop when knowing element index is required,
but with `ezy::enumerate` these workarounds can be forgotten.

How not to do:
```cpp
for (size_t i = 0; i < range.size(); ++i)
{
  const auto& element = range[i];
  ...
}

size_t i = 0;
for (const auto element& : range)
{
  ...
  ++i;
}
```

How to do:
```cpp
for (const auto& [i, element] : ezy::enumerate(range))
{...}
```

As you can see it works flawlessly with c++17 structured binding.

So how to print the enumerated results?

```cpp
const auto print_with_number = [](const auto& element)
{
  const auto& [i, word] = element;
  std::cout << i << ". " << word << "\n";
};

ezy::for_each(ezy::enumerate(long_words), print_with_number);
```

Output:
```cpp
0. looong
1. looooong
2. with, comma, and, space
```

Maybe someone finds it prettier to use `for` loop rather than `ezy::for_each` here, and nothing is wrong with
that.

## How humans enumerate

Computers starts counting by 0, but human beings prefer by 1. To solve this issue we should disassemble
`enumerate` a bit. We have already known `zip` as part of `ezy::iterable::feature` it takes ranges and yields
one element from each in every iteration.

There is another tool here `ezy::iterate`: it takes at least one parameter and generates its successors
endlessly. In our case `ezy::iterate(0)` will return a lazy infinite range of `0, 1, 2, 3, 4, 5, 6`.
The second parameter specifies how to generate the next element from the previous, by default it is incremented.

So logically following things have the same meaning:

```cpp
ezy::enumerate(range) <~~~> ezy::zip(ezy::iterate(0), range)
```

Therefore changing the starting value to `1`:

```cpp
  ezy::for_each(ezy::zip(ezy::iterate(1), long_words), print_with_number);
```

outputs:

```cpp
1. looong
2. looooong
3. with, comma, and, space
```



