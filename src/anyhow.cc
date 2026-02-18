// 🌺💜 Violet: Extended C++ standard library
// Copyright (c) 2025-2026 Noelware, LLC. <team@noelware.org>, et al.
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

#include <violet/Support/Terminal.h>
#include <violet/anyhow.h>

using violet::String;
using violet::anyhow::Error;
using violet::terminal::Style;
using violet::terminal::Styled;

#if VIOLET_REQUIRE_STL(202302L)
namespace {

template<typename... Args>
inline void eprint(std::format_string<Args...> fmt, Args&&... args)
{
    std::print(std::cerr, fmt, VIOLET_FWD(Args, args)...);
}

template<typename... Args>
inline void eprintln(std::format_string<Args...> fmt, Args&&... args)
{
    std::println(std::cerr, fmt, VIOLET_FWD(Args, args)...);
}

} // namespace
#endif

Error::node_t::node_t() noexcept
    : Object(nullptr)
    , VTable(nullptr)
    , Size(0)
    , Next(nullptr)
{
}

Error::node_t::node_t(node_t&& other) noexcept
    : Object(std::exchange(other.Object, nullptr))
    , VTable(std::exchange(other.VTable, {}))
    , Size(std::exchange(other.Size, 0))
    , Location(std::exchange(other.Location, {}))
    , Next(std::exchange(other.Next, nullptr))
{
}

auto Error::node_t::operator=(node_t&& other) noexcept -> node_t&
{
    if (this != &other) {
        if (this->Object != nullptr && this->VTable.Destruct != nullptr) {
            this->VTable.Destruct(this->Object);
        }

        delete this->Next;

        this->Location = std::exchange(other.Location, {});
        this->Object = std::exchange(other.Object, nullptr);
        this->VTable = std::exchange(other.VTable, {});
        this->Size = std::exchange(other.Size, 0);
        this->Next = std::exchange(other.Next, nullptr);
    }

    return *this;
}

Error::node_t::~node_t() noexcept
{
    if (this->Object != nullptr && this->VTable.Destruct != nullptr) {
        this->VTable.Destruct(this->Object);
        this->Object = nullptr;
    }

    delete this->Next;
}

Error::Error(Error&& other) noexcept
    : n_node(std::exchange(other.n_node, nullptr))
{
}

auto Error::operator=(Error&& other) noexcept -> Error&
{
    if (this != &other) {
        delete this->n_node;
        this->n_node = std::exchange(other.n_node, nullptr);
    }

    return *this;
}

Error::~Error() noexcept
{
    if (this->n_node != nullptr) {
        delete this->n_node;
        this->n_node = nullptr;
    }
}

auto Error::Context(Error context) noexcept -> Error
{
    if (context.n_node == nullptr) {
        return VIOLET_MOVE(*this);
    }

    if (this->n_node == nullptr) {
        return context;
    }

    Error out{};
    out.n_node = VIOLET_MOVE(context.n_node);

    node_t* tail = out.n_node;
    while (tail->Next != nullptr) {
        tail = tail->Next;
    }

    tail->Next = VIOLET_MOVE(this->n_node);
    this->n_node = nullptr;

    return out;
}

#if VIOLET_REQUIRE_STL(202302L)
namespace {

template<typename... Args>
void eprintColoured(
    bool colors, [[maybe_unused]] violet::terminal::Style style, std::format_string<Args...> fmt, Args&&... args)
{
    if (!colors) {
        eprint(fmt, VIOLET_FWD(Args, args)...);
        return;
    }

    auto styled = violet::terminal::Styled(std::format(fmt, VIOLET_FWD(Args, args)...), style);
    eprint("{}", styled.Paint());
}

template<typename... Args>
void eprintlnColoured(
    bool colors, [[maybe_unused]] violet::terminal::Style style, std::format_string<Args...> fmt, Args&&... args)
{
    if (!colors) {
        eprintln(fmt, VIOLET_FWD(Args, args)...);
        return;
    }

    auto styled = violet::terminal::Styled(std::format(fmt, VIOLET_FWD(Args, args)...), style);
    eprintln("{}", styled.Paint());
}

} // namespace
#endif

constexpr auto kRedBold = Style::RGB<91, 0, 0>().Bold();

void Error::Print() noexcept
{
    auto colors = violet::terminal::ColoursEnabled(terminal::StreamSource::Stderr);
    auto window = violet::terminal::QueryWindowInfo().UnwrapOr({ .Columns = 80, .Rows = 0 });

#if VIOLET_REQUIRE_STL(202302L)
    eprintln(
        "{:━^{}}", colors ? violet::terminal::Styled(" Error: ", kRedBold).Paint() : " Error: ", window.Columns + 25);
#else
    std::cerr << std::format(
        "{:━^{}}", colors ? violet::terminal::Styled(" Error: ", kRedBold).Paint() : " Error: ", window.Columns + 25)
              << '\n';
#endif

    Vec<const node_t*> stack;
    for (auto* node = this->n_node; node != nullptr; node = node->Next) {
        stack.push_back(node);
    }

    UInt index = 0;
    for (auto it = stack.rbegin(); it != stack.rend(); ++it, ++index) {
        const node_t* node = *it;
        VIOLET_DEBUG_ASSERT(node != nullptr, "invalid invariant: `node` shouldn't be null");
        VIOLET_DEBUG_ASSERT(node->Object != nullptr,
            std::format("missing object in child node of {:p}", static_cast<const void*>(node)));
        VIOLET_DEBUG_ASSERT(node->VTable.Message != nullptr, "invalid invariant reached: vtable missing `Message()'");

#if VIOLET_REQUIRE_STL(202302L)
        if (index == 0) {
            eprintln("{} [{}:{}:{}]", node->VTable.Message(node->Object), node->Location.file_name(),
                node->Location.line(), node->Location.column());
        } else {
            eprint("    ~> #");
            eprintColoured(colors, Style{}.Bold(), "{}", index - 1);

            eprintln(": {}", node->VTable.Message(node->Object));
        }
#else
        if (index == 0) {
            std::cerr << node->VTable.Message(node->Object) << " [" << node->Location.file_name() << ':'
                      << node->Location.line() << ':' << node->Location.column() << "]\n";
        } else {
            std::cerr << "    ~> #";
            if (colors) {
                std::cerr << Styled<UInt>(index - 1, Style{}.Bold());
            } else {
                std::cerr << index - 1;
            }

            std::cerr << ": " << node->VTable.Message(node->Object) << '\n';
        }
#endif
    }
}
