# Design Principles

## General Interface Design Principles

### Correctness

1. The code should be correct or refused by the compiler. (not there yet)
2. If the compilation failed, the compilation error should explain the reason. (far from this)
  - Preferably it should recommend a fix.
3. The generated code should be reasonably efficient.
4. Correctness is preferred over performance (good by default)
  - Advanced methods may be provided as non-defaults (opt-in).
5. The code should be readable (readable != familiar)

### Usability

1. The library should smoothly integrate with existing codebases. (it's not a framework)
2. The interface should prefer immutability as much as possible, while considering conformance with the
   existing C++ routine.

## ezy-specific goals

### Main goals

- experimenting
- provide concise and readable syntax
- advocate compile-time errors over runtime bugs

### Other goals (not there yet)

- readable compilation error messages
- does not hurt runtime performance
- does not bloat compilation time too much

### About the library

- mainly focuses on c++17
  - there is no plan to updgrade to c++20 yet
  - some level of compatibility with c++14 is considered
- is built on the top of the standard library. Basically follows conventions by the standard.
- gets some inspiration from other langages and libraries, like Rust, Scala or Haskell.
- everything is experimental and nothing is production ready!
- There is also a nested `experimental` namespace containing drafts and raw ideas which are are very likely to
  be changed or removed in the future.
- does not favor macro usage
