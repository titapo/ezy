#include <ezy/strong_type.h>
#include <ezy/features/iterable.h>
#include <ezy/string.h>

#include <vector>
#include <string>
#include <iostream>

using Names = ezy::strong_type<std::vector<std::string>, struct NamesTag, ezy::features::iterable>;
using Scores = ezy::strong_type<std::vector<int>, struct PointsTag, ezy::features::iterable>;

bool test_passed(int score)
{ return score >= 25; }

const auto display = [](const std::string name, int score) {
  return name + " (" + ezy::to_string(score) + ")";
};

const auto display_with_name_and_score = [](const auto& name_with_score) {
  return std::apply(display, name_with_score);
};

const auto print_line = [](const auto& line)
{
  std::cout << line << "\n";
};

auto number_of_passed_tests(const Scores& scores)
{
  return scores
    .filter(test_passed)
    .size();
}

auto who_passed(const Scores& scores, const Names& names)
{
  return names
    .zip(scores)
    .filter([](const auto& name_with_score) {
        return test_passed(std::get<1>(name_with_score));
        })
    .map(display_with_name_and_score);
}

#include <ezy/experimental/compose.h>
auto who_passed_experimental(const Scores& scores, const Names& names)
{
  return names
    .zip(scores)
    .filter(
        ezy::experimental::compose(ezy::pick_second, test_passed)
        )
    .map(display_with_name_and_score);
}

int main()
{
  Names names{"Alice", "Bob", "Cecil", "David"};
  Scores scores{10, 35, 23, 29};

  print_line("names:");
  names.for_each(print_line);

  const std::string prefix = " - ";
  auto prepend = [&](const std::string& s) { return prefix + s; };
  std::cout << "names:\n";
  names.map(prepend).for_each(print_line);

  std::cout << "number_of_passed_tests: " << number_of_passed_tests(scores) << "\n";

  print_line("all names with scores:");
  names.zip_with(display, scores).map(prepend).for_each(print_line);

  std::cout << "Who passed:\n" << who_passed(scores, names).map(prepend).join("\n");
  std::cout << "Who passed:\n" << who_passed_experimental(scores, names).map(prepend).join("\n");
  
  return 0;
}
