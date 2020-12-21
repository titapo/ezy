# Components

## Main

- [strong_type](../tutorial/01_strong_type.md): yet another strong type library (originally inspired by [NamedType](https://github.com/joboccara/NamedType))
  - traits for strong types
  - features for inheriting underlying types' capabilities (not exhaustive)
  - introduces the idea of extending features (eg. [iterable feature](../tutorial/iterable)) and so [extended types](../tutorial/04_extended_type.md)
- strong vocabulary types
  - on the top of the strong types wraps some commonly used standard types and extends them to make them extremely powerful (eg [optional](../tutorial/06_optional.md), variant, [result](../tutorial/07_result.md))
- algorithms

## (More) Experimental

- `typelist_traits`: a typelist library using (working with types)
  - `tuple_traits` for some specialization for `std::tuple`
- (experimental) tuple algorithms: defines typical algorithms "iterating over" tuple values
- (experimental) keeper: generalization of reference wrapper to help express intentions about ownership and
  reference ([Universal reference wrapper](https://www.fluentcpp.com/2020/06/26/implementing-a-universal-reference-wrapper/))
- (experimental) nullable: self-contained optional-like type without space overhead
- (experimental) function(al) utilities: *curry* and *compose*
