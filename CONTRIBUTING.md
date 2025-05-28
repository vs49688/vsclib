# Contribution

## Style guide

Use FFmpeg's ([here](https://ffmpeg.org/developer.html#toc-Code-formatting-conventions)), with the following
exceptions:

* Don't add spaces after conditional/loop statements, i.e. `if`, `for`, `while`, etc.

## Guidelines

* Minimise dependencies - i.e. don't pull in a behemoth like OpenSSL or libcurl.
  The C standard library is fine.

* Avoid using `errno` to report errors. Return a `VSC_ERROR(ESOMETHING)` value instead.
    - Exceptions:
        - The `vsc_sys_*()` functions. These are direct system wrappers meant
          to avoid `#ifdef`s on Windows/Linux/other, and as such are allowed to use `errno`.
        - `#ifdef`'d Platform-specific convenience functions where:
            - `errno` is ALWAYS available on said platform, and
            - Returning an error and makes the API significantly more cumbersome.

          This behaviour MUST be explicitly documented. Prefer returning a `VSC_ERROR` value, if possible.
    - A lot of existing code uses `errno` and is gradually being fixed.

* If a function allocates memory, it should do it in a fashion compatible with linear
  allocators, i.e. like a stack (LIFO). Reallocations (`VSC_ALLOC_REALLOC`, `vsc_xrealloc`, et. al.)
  are preferred for this purpose.

* C++ _may_ be used internally, but _only_ for language features if they provide significant code simplification for a
  task at hand (e.g. using templates to avoid a nasty macro).
    - It must _not_ be exposed via the API.
    - No STL (except `<type_traits>`), exceptions, or RTTI are permitted.
    - Try to keep it at C++17.
    - QoL C++ things may go in `vscpplib`.
