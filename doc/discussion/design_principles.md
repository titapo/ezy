# Interface Design Principles

## Correctness

1. The code should be correct or refused by the compiler. (not there yet)
2. If the compilation failed, the compilation error should explain the reason. (far from this)
  - Preferably it should recommend a fix.
3. The generated code should be reasonably efficient.
4. Correctness is preferred over performance (good by default)
  - Advanced methods may be provided as non-defaults (opt-in).
5. The code should be readable (readable != familiar)

## Usability

1. The library should smoothly integrate with existing codebases. (it's not a framework)
2. The interface should prefer immutability as much as possible, while considering conformance with the
   existing C++ routine.
