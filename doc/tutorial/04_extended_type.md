# Extended type

We have the following function:

```cpp
std::vector<std::string> get_words_longer_than(const std::vector<std::string>& words, size_t treshold)
{
  std::vector<std::string> result;
  for (const auto& word : words)
  {
    if (word.size() > treshold)
      result.push_back(word);
  }

  return result;
}
```

(It is part of a public API, so the signature cannot be changed.)

The task is the following: implement

```cpp
size_t get_number_of_words_longer_than(const std::vector<std::string>& words, size_t treshold);
```

One could implement it as
```cpp
return get_words_longer_than(words, treshold);
/* or */
size_t result{0};
for (const auto& word : words)
{
  if (word.size() > treshold)
    ++result;
}

return result;
```

The first one reuses code, but it copies potentially a lot of elements just to return how many elements were copied. The second one has more efficient in runtime, but the body is almost a copy of `get_words_longer_than`.

## First step: refactor

`get_words_longer_than` is not so convenient, `iterable` feature can make it more expressible. Because of the
public API cannot be changed, it must be solved in the function body.

##### Attempt 1
```cpp
ezy::strong_type<const std::vector<std::string>, struct WordsTag, ezy::features::iterable>(words)
```
Problem: it copies the `words` vector.

##### Attempt 2
```cpp
ezy::strong_type_reference<const std::vector<std::string>, struct WordsTag, ezy::features::iterable>(words)
```

`ezy::strong_type_reference` is a convenient way to have a reference to an object, but access it via the
interface of the strong type. That is what we need.

Problem: it is too verbose

##### Attempt 3
```cpp
ezy::make_strong_reference<struct WordsTag, ezy::features::iterable>(words)
```

`make_strong_reference` is a function which deduces the underlying type.

Problem: `WordsTag` is still superfluous. We want to extend `std::vector` interface only, and not really
strengthen its type.

##### Attempt 4 - solution
```cpp
ezy::make_extended_reference<ezy::features::iterable>(words)
```

#### The code

```cpp
std::vector<std::string> get_words_longer_than(const std::vector<std::string>& words, size_t treshold)
{
  return ezy::make_extended_reference<ezy::features::iterable>(words)
    .filter([treshold](const auto& word) { return word.size() > treshold; })
    .to<std::vector<std::string>>();
}
```

`filter` was covered in the previous tutorial: it returns a lazy range of the filtered elements, so we have to
collect (copy) those elements into a vector.

The new function can be implemented very similar, only the last line has to be changed

```cpp
size_t get_number_of_words_longer_than(const std::vector<std::string>& words, size_t treshold)
{
  return ezy::make_extended_reference<ezy::features::iterable>(words)
    .filter([treshold](const auto& word) { return word.size() > treshold; })
    .size();
}
```

Although we can easier reason about this code than the original version, it is still duplicated code.

## Refactor further

What if we would implement a private function to do the common job?

```cpp
auto longer_words(const std::vector<std::string>& words, size_t treshold)
{
  return ezy::make_extended_reference<ezy::features::iterable>(words)
    .filter([treshold](const auto& word) { return word.size() > treshold; });
}
```

The return type is auto, because it returns an opaque type which represents a filtered lazy view. Be careful:
it returns a view of a reference. Now it comes from the outer scope, but in general this can cause problems.
The other dangerous point is the lambda capture: Here it is necessary to be captured by value.

```cpp
std::vector<std::string> get_words_longer_than(const std::vector<std::string>& words, size_t treshold)
{
  return longer_words(words, threshold).to<std::vector<std::string>>();
}

size_t get_number_of_words_longer_than(const std::vector<std::string>& words, size_t treshold)
{
  return longer_words(words, threshold).size();
}
```

