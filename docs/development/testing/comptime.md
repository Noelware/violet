---
title: "Development | Compile-Time Driven Testing"
description: "A guide on how to do compile-time-based driven testing for constexpr / consteval-capable objects"
---

Any type or function in the Violet frameworks that are meant to work in a `constexpr` or `consteval` context should also be exercised at compile-time, not just at runtime. Regular `TEST(...)` macros inside of test files from GoogleTest only proves it works at runtime, it doesn't say nothing about whether the same code path still compiles and evaluates correctly at *compile-time*.

Compile-time driven tests close that gap by forcing the exact same operations through `consteval` functions inside of anonymous namespaces and `static_assert` so that changes that accidentally breaks `constexpr`-friendless (e.g., introducing non-constexpr calls, relying on UB, `reintepret_cast`s, etc.) fails CI / local tests instead of silently regressing.

## The Pattern
Compile-time tests live in the same `test.cc` file as their runtime counterparts, inside an anonymous namespace [usually] at the bottom of the file. Each check is a `consteval` function returning `bool` paired with a `static_assert` that calls it.

A perfect example is [`tests/container/result/Err.test.cc`](../../../tests/container/result/Err.test.cc):
```cpp
namespace {

// `consteval` is preferred due to it being only executed and usuable in compile-time contexts
// like `static_assert`.
//
// `bool` is usually the return value so that it can be used in `static_assert`. When it fails, the
// compile-time errors will give you a better understanding why it couldn't be `constexpr`-friendly.
consteval auto constructAndAccess() noexcept -> bool
{
    constexpr Err error(42);
    return error.Error() == 42;
}

static_assert(constructAndAccess());

} // namespace
```

### Q1. Why `consteval` and not `constexpr`?
C++'s **consteval** forces the function to be evaluated at compile-time on every call. A `constexpr` can and will fall-back to running the test at runtime if it's called from a non-constexpr context, which would defeat the point of "compile-time driven testing".

### Q2. Why is there an anonymous namespace?
Helper functions, when written, have zero reason to be visible outside of the current translation unit, and short and generic names like this would risk clashing across test file. Using an anonymous namespace gives them internal linkage for free without having to invent a per-file prefix.

### Q3. Couldn't you just write a single `consteval` function instead of smaller ones?
Each `consteval` function you write should just assert a singular case instead of multiple cases so that it is easy to reason about, similar to splitting up runtime tests using GoogleTest's `TEST(...)` macros you see in test files.

## When to do this?
Compile-time driven tests should be only used in contexts where it is meant to work in `constexpr` contexts. If a method isn't `constexpr` (i.e, I/O, allocations, byte manipulation with `reintepret_cast`, etc.), it is not recommended. Runtime tests are sufficient enough.
