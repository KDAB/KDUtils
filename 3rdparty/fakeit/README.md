# FakeIt

FakeIt 2.4.1 (from <https://raw.githubusercontent.com/eranpeer/FakeIt/refs/tags/2.4.1/single_header/doctest/fakeit.hpp>)
with a fix for stricter compilation on Clang >=19.

Line 7924 changed from:

```cpp
ParamWalker<N - 1>::template Assign(arg_vals, std::forward<arglist>(args)...);
```

```cpp
ParamWalker<N - 1>::Assign(arg_vals, std::forward<arglist>(args)...);
```

See <https://github.com/eranpeer/FakeIt/issues/348>

After FakeIt 2.5.0 is released we can go back to using the vanilla version.
