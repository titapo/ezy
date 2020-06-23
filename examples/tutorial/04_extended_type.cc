#include <vector>
#include <string>

namespace old_impl
{
  std::vector<std::string> get_words_longer_than(const std::vector<std::string>& words, size_t treshold);
  size_t get_number_of_words_longer_than(const std::vector<std::string>& words, size_t treshold);

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

  size_t get_number_of_words_longer_than(const std::vector<std::string>& words, size_t treshold)
  {
    return get_words_longer_than(words, treshold).size();

    /* or
    size_t result{0};
    for (const auto& word : words)
    {
      if (word.size() > treshold)
        ++result;
    }

    return result;
    */
  }
}

#include <ezy/strong_type.h>
#include <ezy/features/iterable.h>

namespace new_impl
{
  std::vector<std::string> get_words_longer_than(const std::vector<std::string>& words, size_t treshold);
  size_t get_number_of_words_longer_than(const std::vector<std::string>& words, size_t treshold);

  auto longer_words(const std::vector<std::string>& words, size_t treshold)
  {
      return ezy::make_extended_reference<ezy::features::iterable>(words)
      .filter([treshold](const auto& word) { return word.size() > treshold; });
  }

  std::vector<std::string> get_words_longer_than(const std::vector<std::string>& words, size_t treshold)
  {
    return
      //ezy::strong_type_reference<const std::vector<std::string>, struct WordsTag, ezy::features::iterable>(words)
      //ezy::make_strong_reference<struct WordsTag, ezy::features::iterable>(words)
      ezy::make_extended_reference<ezy::features::iterable>(words)
      .filter([treshold](const auto& word) { return word.size() > treshold; })
      .to<std::vector<std::string>>();
  }

  size_t get_number_of_words_longer_than(const std::vector<std::string>& words, size_t treshold)
  {
    return get_words_longer_than(words, treshold).size();

    /* or
    size_t result{0};
    for (const auto& word : words)
    {
      if (word.size() > treshold)
        ++result;
    }

    return result;
    */
  }
}

int main()
{
  const std::vector<std::string> words{"a", "bb", "looong", "looooong"};

}
