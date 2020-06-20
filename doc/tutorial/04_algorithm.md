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





