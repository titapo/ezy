#ifndef EZY_TESTS_JOIN_AS_STRINGS_H_INCLUDED
#define EZY_TESTS_JOIN_AS_STRINGS_H_INCLUDED

#include <ezy/algorithm/join.h>
#include <ezy/algorithm/transform.h>
#include <ezy/string.h>

template <typename Range>
std::string
join_as_strings(Range&& range, std::string_view separator = "")
{
  return ezy::join(ezy::transform(std::forward<Range>(range), ezy::to_string), separator);
}


#endif
