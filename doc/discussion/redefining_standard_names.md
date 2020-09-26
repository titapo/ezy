# Redefining standard names

Sometimes `ezy` redefines some identifiers from the standard: for example `ezy::optional` is an extended type
for `std::optional`. In several cases they can replaced by the other, usually they provide some additional
value.

Sometimes they can work as polyfill: `ezy::invoke` is marked `constexpr` while `std::invoke` does not before
c++20. This applies to `ezy::remove_cvref`, it does not exist in c++17. If you access newer version then
`std` names should be used.

`std::to_string` identifies an overload set, which is very inconvenient to be passed to a higher order
function, so `ezy::to_string` wraps it in a function object. Additionally it is a customization point object
(CPO), so if there is a `to_string(my_type)` function which can be found by argument dependent lookup (ADL),
`ezy::to_string` will find it too.

And some cases `ezy` use different names for the same thing. `ezy::transform` is really similar to
`std::transform` (or more likely to `std::ranges::transform` modulo concepts), but in iterable feature it can
be accessed as `.map`. Similarly `ezy::result` is the counterpart of the proposed `std::expected`. Those
differences are usually justified by other languages or libraries.
