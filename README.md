ezy
===

[![Build Status](https://travis-ci.com/titapo/ezy.svg?branch=master)](https://travis-ci.com/titapo/ezy)

`ezy` is an experimental library, it is a collection of some best practices and syntactic sugars, focuses on
code readability and safety without hurting performance.

## Main components

- [strong_type](doc/tutorial/01_strong_type.md): yet another strong type library (originally inspired by [NamedType](https://github.com/joboccara/NamedType))
  - traits for strong types
  - features for inheriting underlying types' capabilities (not exhaustive)
  - introduces the idea of extending features (eg. [iterable feature](doc/tutorial/03_iterable.md)) and so [extended types](doc/tutorial/04_extended_type.md)
- strong vocabulary types
  - on the top of the strong types wraps some commonly used standard types and extends them to make them extremely powerful (eg [optional](doc/tutorial/06_optional.md), variant, [result](doc/tutorial/07_result.md))
- algorithms

More [components](doc/discussion/components.md).

## Documentation

> Writing documentation is in progress and currently insufficient.

[Install guide](doc/install.md).

There are [tutorials](doc/tutorial/) to learn the basics of `ezy`.

[Background](doc/discussion/) about the design ideas and guidlines of the library.

> [how-to guides](doc/howto/?) ~~can~~ will be found later covering some more advanced topics

> [API Reference](doc/reference/?) is also planned. Until then please inspect unit tests or read in-line comments.
