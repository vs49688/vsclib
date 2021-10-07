# Contribution

## Style guide

Use FFmpeg's ([here](https://ffmpeg.org/developer.html#toc-Code-formatting-conventions)), with the following
exceptions:
* Don't add spaces after conditional/loop statements, i.e. `if`, `for`, `while`, etc.

## Guidelines

* Minimise dependencies - i.e. don't pull in a behemoth like OpenSSL or libcurl.
The C standard library is fine.

* Avoid using `errno` to report errors. Return a `VSC_ERROR(ESOMETHING)` value instead.
  - An exception is the `vsc_sys_*()` functions. These are direct system wrappers meant
    to avoid `#ifdef`s on Windows/Linux/other, and as such are allowed to use `errno`.
  - A lot of existing code uses `errno` and is gradually being fixed.
* If a function allocates memory, it should do it in a fashion compatible with linear
  allocators, i.e. like a stack (LIFO). Reallocations (`VSC_ALLOC_REALLOC`, `vsc_xrealloc`, et. al.)
  are preferred for this purpose.
