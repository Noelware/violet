// 🌺💜 Violet: Extended standard library for C++26
// Copyright (c) 2025 Noelware, LLC. <team@noelware.org> & other contributors
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include "violet/iter/Iterator.h"
#include <utility>

namespace Noelware::Violet {
template<typename Pred>
constexpr auto Position(Pred predicate) noexcept;
}

namespace Noelware::Violet::Iterators {

template<typename T, typename Inner>
concept ValidPipeApplicator = requires(const T& ty, Inner&& inner) {
    { ty.Apply(std::forward<Inner>(inner)) };
};

template<typename T, typename Inner>
concept ValidPipeConsumer = requires(const T& ty, Inner&& inner) {
    { ty.Consume(std::forward<Inner>(inner)) };
};

template<typename Derived>
struct PipeFactory {
    template<typename Inner>
    auto operator()(Inner&& inner) const noexcept
    {
        if constexpr (ValidPipeApplicator<Derived, Inner>) {
            auto* thisObject = static_cast<const Derived*>(this);
            return thisObject->Apply(std::forward<Inner>(inner));
        } else {
            static_assert(sizeof(Derived) == 0, "`PipeFactory` requires `Derived::Apply(Inner)` to be avaliable");
        }
    }

    template<typename Inner>
    friend auto operator|(Inner&& inner, const Derived& factory)
        requires ValidPipeApplicator<Derived, Inner>
    {
        return factory(std::forward<Inner>(inner));
    }
};

template<typename Derived>
struct PipeConsumer {
    template<typename Inner>
    friend auto operator|(Inner&& inner, const Derived& pipe)
        requires ValidPipeConsumer<Derived, Inner>
    {
        return pipe.Consume(std::forward<Inner>(inner));
    }
};

#define MK_PIPE_CONSUMERS_WITH_PREDICATES(NAME)                                                                        \
    template<typename Pred>                                                                                            \
        struct CONCAT(NAME, PipeConsumer) final: PipeConsumer < CONCAT(NAME, PipeConsumer) < Pred >> {                 \
        CONCAT(NAME, PipeConsumer)() = delete;                                                                         \
        CONCAT(NAME, PipeConsumer)(Pred predicate)                                                                     \
            : n_predicate(VIOLET_MOVE(predicate)) {};                                                                  \
                                                                                                                       \
        template<Iterable Impl>                                                                                        \
        auto Consume(Impl&& it) const noexcept                                                                         \
        {                                                                                                              \
            return VIOLET_FWD(Impl, it).NAME();                                                                        \
        }                                                                                                              \
                                                                                                                       \
    private:                                                                                                           \
        Pred n_predicate;                                                                                              \
    };

#define MK_PIPE_CONSUMER_WITHOUT_ARGUMENTS(NAME)                                                                       \
    struct CONCAT(NAME, PipeConsumer) final: PipeConsumer<CONCAT(NAME, PipeConsumer)> {                                \
        CONCAT(NAME, PipeConsumer)() = default;                                                                        \
                                                                                                                       \
        template<Iterable Impl>                                                                                        \
        auto Consume(Impl&& it) const noexcept                                                                         \
        {                                                                                                              \
            return VIOLET_FWD(Impl, it).NAME();                                                                        \
        }                                                                                                              \
    };

#define MK_PIPE_CONSUMER_ARG1(NAME, TYPE)                                                                              \
    struct CONCAT(NAME, PipeConsumer) final: PipeConsumer<CONCAT(NAME, PipeConsumer)> {                                \
        CONCAT(NAME, PipeConsumer)() = delete;                                                                         \
        CONCAT(NAME, PipeConsumer)(TYPE value)                                                                         \
            : n_value(VIOLET_MOVE(value))                                                                              \
        {                                                                                                              \
        }                                                                                                              \
                                                                                                                       \
        template<Iterable Impl>                                                                                        \
        auto Consume(Impl&& it) const noexcept                                                                         \
        {                                                                                                              \
            return VIOLET_FWD(Impl, it).NAME(this->n_value);                                                           \
        }                                                                                                              \
                                                                                                                       \
    private:                                                                                                           \
        TYPE n_value;                                                                                                  \
    };

MK_PIPE_CONSUMERS_WITH_PREDICATES(Position);
MK_PIPE_CONSUMERS_WITH_PREDICATES(Inspect);
MK_PIPE_CONSUMERS_WITH_PREDICATES(Find);
MK_PIPE_CONSUMERS_WITH_PREDICATES(Any);

MK_PIPE_CONSUMER_WITHOUT_ARGUMENTS(Count);
MK_PIPE_CONSUMER_ARG1(AdvanceBy, usize);
MK_PIPE_CONSUMER_ARG1(Nth, usize);

#undef MK_PIPE_CONSUMERS_WITH_PREDICATES
#undef MK_PIPE_CONSUMER_WITHOUT_ARGUMENTS
#undef MK_PIPE_CONSUMER_ARG1

// This lives inside of the `Iterators` namespace so it doesn't conflict with [`Noelware::Violet::Any`] (alias for
// `std::any`).
template<typename Pred>
constexpr auto Any(Pred predicate) noexcept
{
    return Iterators::AnyPipeConsumer(predicate);
}

} // namespace Noelware::Violet::Iterators

namespace Noelware::Violet {

#define MK_PIPE_FUNCTION_WITH_PREDICATE(NAME)                                                                          \
    template<typename Pred>                                                                                            \
    constexpr auto NAME(Pred predicate) noexcept                                                                       \
    {                                                                                                                  \
        return Iterators::CONCAT(NAME, PipeConsumer)(predicate);                                                       \
    }

#define MK_PIPE_FUNCTION_ARG1(NAME, TYPE)                                                                              \
    constexpr auto NAME(TYPE value) noexcept                                                                           \
    {                                                                                                                  \
        return Iterators::CONCAT(NAME, PipeConsumer)(value);                                                           \
    }

#define MK_PIPE_FUNCTION_ARG0(NAME) inline constexpr auto NAME = Iterators::CONCAT(NAME, PipeConsumer)();

MK_PIPE_FUNCTION_WITH_PREDICATE(Position);
MK_PIPE_FUNCTION_WITH_PREDICATE(Inspect);
MK_PIPE_FUNCTION_WITH_PREDICATE(Find);
MK_PIPE_FUNCTION_ARG1(AdvanceBy, usize);
MK_PIPE_FUNCTION_ARG1(Nth, usize);
MK_PIPE_FUNCTION_ARG0(Count);

#undef MK_PIPE_FUNCTION_WITH_PREDICATE
#undef MK_PIPE_FUNCTION_ARG1
#undef MK_PIPE_FUNCTION_ARG0

} // namespace Noelware::Violet
