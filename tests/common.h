#ifndef TESTS_COMMON_H_INCLUDED
#define TESTS_COMMON_H_INCLUDED

struct move_only
{
  int i;
  explicit move_only(int i) : i(i) {}

  move_only(const move_only&) = delete;
  move_only& operator=(const move_only&) = delete;

  move_only(move_only&&) = default;
  move_only& operator=(move_only&&) = default;
};

struct non_transferable
{
  int i;
  explicit non_transferable(int i) : i(i) {}

  non_transferable(const non_transferable&) = delete;
  non_transferable& operator=(const non_transferable&) = delete;

  non_transferable(non_transferable&&) = delete;
  non_transferable& operator=(non_transferable&&) = delete;
};


#endif
