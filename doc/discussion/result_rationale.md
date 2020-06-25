# Result rationale


Optionals are really useful to express presence of a value. Although sometimes it could be useful to have some
additional info about absence. In C there is a common pattern to have an error code as a return value and an
(in-)out parameter which initializes the value.

```cpp
errorcode init_foo(foo* ptr, const foo_init_params* params);
```

And `ptr` will be valid only if errorcode returned a non-error value (whatever it means). Just to be clear:
nothing forces the programmer to check the errorcode before accesing `ptr`, it easily can be ignored. It is
very similar to calling `.value()` on an optional without having any check beforehand.

One attempt to have a better interface (now in C++):

```cpp
foo create_foo(const foo_init_params& params);
```

If you are wondering what happened with the error code, it is still there, just be thrown as an exception. So
only the properly written documentation or the actual implementation can be considered to decide what kind of
exceptions should be caught. Unfortunately the function signature itself does not carries the fact that this
function communicates with exceptions. While the exceptions cannot be ignored in runtime (they will terminate
your program if they were not handled), they can be ignored at compile time (not having exception handing code
at all), which might result in unexpected runtime behaviour.

Someone else would try use `ezy::optional`:

```cpp
ezy::optional<foo> create_foo(const foo_init_params& params);
```

Well this signature clearly shows the intent: caller might not get anything back. However the reason is
disappeared now. So we need something more.

Consider the following signature:

```cpp
ezy::result<foo, errorcode> create_foo(const foo_init_params& params);
```

`ezy::result` is very similar to `ezy::optional`, just it holds `errorcode` instead of `nullopt`. So this
`create_foo` returns *either* a `foo` instance or an `errorcode`. (Technically it is an `extended_type` over a
double element `std::variant`).

The main point here that the programmer has to "get through" the `result` to access the desired `foo`
instance. Writing if-else branches by hand could be very tedious and still error prone. Fortunately
`ezy::result` uses the same toolset as `ezy::optional` does, like `.map`, `.and_then` etc.
