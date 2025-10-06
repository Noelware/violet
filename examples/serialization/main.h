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

#include "violet/container/Optional.h"
#include "violet/serialization/Metadata.h"
#include "violet/violet.h"

using namespace Noelware::Violet; // NOLINT(google-build-using-namespace,google-global-names-in-headers)

struct Address final {
    String Address1;
    Optional<String> Address2;
    String City;
    String State;
    uint32 PostalCode;

    auto ToString() const noexcept -> String
    {
        String fmt = std::format("{}", Address1);
        Address2.Inspect([&](const String& addr) { fmt += std::format(" ({})", addr); });

        fmt += '\n';
        fmt += std::format("  {}, {} {}", City, State, PostalCode);

        return fmt;
    }
};

// clang-format off
MARK_SERIALIZABLE(
    Address,
    "org.noelware.violet.examples.Address",

    Field("first_address", &Address::Address1),
    Field("second_address", &Address::Address2),
    Field("city", &Address::City),
    Field("state", &Address::State),
    Field("postal_code", &Address::PostalCode)
);
// clang-format on

struct User final {
    String FullName;
    uint8 Age;
    Address Address;

    auto ToString() const noexcept -> String
    {
        return std::format("User {} (age {}) that lives at\n  {}", FullName, Age, Noelware::Violet::ToString(Address));
    }
};

// clang-format off
MARK_SERIALIZABLE(
    User,
    "org.noelware.violet.examples.User",

    Field("full_name", &User::FullName),
    Field("age", &User::Age),
    Field("address", &User::Address)
);
// clang-format on
