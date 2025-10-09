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
#include <sstream>

using namespace Noelware::Violet; // NOLINT(google-build-using-namespace,google-global-names-in-headers)

/// A United States state abbreviated.
enum struct State : uint8 {
    /// Alabama
    AL,

    /// Alaska
    AK,

    /// Arizona
    AZ,

    /// Arkansas
    AR,

    /// California
    CA,

    /// Colorado
    CO,

    /// Connecticut
    CT,

    /// Delaware
    DE,

    /// Florida
    FL,

    /// Georgia
    GA,

    /// Hawaii
    HI,

    /// Idaho
    ID,

    /// Illinois
    IL,

    /// Indiana
    IN,

    /// Iowa
    IA,

    /// Kansas
    KS,

    /// Kentucky
    KY,

    /// Louisiana
    LA,

    /// Maine
    ME,

    /// Maryland
    MD,

    /// Massachusetts
    MA,

    /// Michigan
    MI,

    /// Minnesota
    MN,

    /// Mississippi
    MS,

    /// Missouri
    MO,

    /// Montana
    MT,

    /// Nebraska
    NE,

    /// Nevada
    NV,

    /// New Hampshire
    NH,

    /// New Jersey
    NJ,

    /// New Mexico
    NM,

    /// New York
    NY,

    /// North Carolina
    NC,

    /// North Dakota
    ND,

    /// Ohio
    OH,

    /// Oklahoma
    OK,

    /// Oregon
    OR,

    /// Pennsylvaina
    PA,

    /// Rhode Island
    RI,

    /// South Carolina
    SC,

    /// South Dakota
    SD,

    /// Tennessee
    TN,

    /// Texas
    TX,

    /// Utah
    UT,

    /// Vermont
    VT,

    /// Virginia
    VA,

    /// Washington
    WA,

    /// West Virginia
    WV,

    /// Wisconsin
    WI,

    /// Wyoming
    WY
};

VIOLET_TO_STRING(const State&, state, {
    switch (state) {
    case State::AL:
        return "Alabama";

    case State::AK:
        return "Alaska";

    case State::AZ:
        return "Arizona";

    case State::AR:
        return "Arkansas";

    case State::CA:
        return "California";

    case State::CO:
        return "Colorado";

    case State::CT:
        return "Connecticut";

    case State::DE:
        return "Delaware";

    case State::FL:
        return "Florida";

    case State::GA:
        return "Georgia";

    case State::HI:
        return "Hawaii";

    case State::ID:
        return "Idaho";

    case State::IL:
        return "Illinois";

    case State::IN:
        return "Indiana";

    case State::IA:
        return "Iowa";

    case State::KS:
        return "Kansas";

    case State::KY:
        return "Kentucky";

    case State::LA:
        return "Louisiana";

    case State::ME:
        return "Maine";

    case State::MD:
        return "Maryland";

    case State::MA:
        return "Massachusetts";

    case State::MI:
        return "Michigan";

    case State::MN:
        return "Minnesota";

    case State::MS:
        return "Mississippi";

    case State::MO:
        return "Missouri";

    case State::MT:
        return "Montana";

    case State::NE:
        return "Nebraska";

    case State::NV:
        return "Nevada";

    case State::NH:
        return "New Hampshire";

    case State::NJ:
        return "New Jersey";

    case State::NY:
        return "New York";

    case State::NC:
        return "North Carolina";

    case State::NM:
        return "New Mexico";

    case State::ND:
        return "North Dakota";

    case State::OH:
        return "Ohio";

    case State::OK:
        return "Oklahoma";

    case State::OR:
        return "Oregon";

    case State::PA:
        return "Pennsylvaina";

    case State::RI:
        return "Rhode Island";

    case State::SC:
        return "South Carolina";

    case State::SD:
        return "South Dakota";

    case State::TX:
        return "Texas";

    case State::TN:
        return "Tennessee";

    case State::UT:
        return "Utah";

    case State::VT:
        return "Vermont";

    case State::VA:
        return "Virginia";

    case State::WA:
        return "Washington";

    case State::WV:
        return "West Virginia";

    case State::WI:
        return "Wisconsin";

    case State::WY:
        return "Wyoming";
    }
});

// clang-format off
MARK_ENUM_SERIALIZABLE(State,
    {State::AL, "Alabama"},
    {State::AK, "Alaska"},
    {State::AZ, "Arizona"},
    {State::AR, "Arkansas"},
    {State::CA, "California"},
    {State::CO, "Colorado"},
    {State::CT, "Connecticut"},
    {State::DE, "Delaware"},
    {State::FL, "Florida"},
    {State::GA, "Georgia"},
    {State::HI, "Hawaii"},
    {State::ID, "Idaho"},
    {State::IL, "Illinois"},
    {State::IN, "Indiana"},
    {State::IA, "Iowa"},
    {State::KS, "Kansas"},
    {State::KY, "Kentucky"},
    {State::LA, "Louisiana"},
    {State::ME, "Maine"},
    {State::MD, "Maryland"},
    {State::MA, "Massachusetts"},
    {State::MI, "Michigan"},
    {State::MN, "Minnesota"},
    {State::MO, "Missouri"},
    {State::MT, "Montana"},
    {State::NE, "Nebraska"},
    {State::NV, "Nevada"},
    {State::NH, "New Hampshire"},
    {State::NJ, "New Jersey"},
    {State::NY, "New York"},
    {State::NC, "North Carolina"},
    {State::NM, "New Mexico"},
    {State::ND, "North Dakota"},
    {State::OH, "Ohio"},
    {State::OK, "Oklahoma"},
    {State::OR, "Oregon"},
    {State::PA, "Pennsylvaina"},
    {State::RI, "Rhode Island"},
    {State::SC, "South Carolina"},
    {State::SD, "South Dakota"},
    {State::TX, "Texas"},
    {State::TN, "Tennessee"},
    {State::UT, "Utah"},
    {State::VT, "Vermont"},
    {State::VA, "Virginia"},
    {State::WA, "Washington"},
    {State::WV, "West Virginia"},
    {State::WI, "Wisconsin"},
    {State::WY, "Wyoming"}
);
// clang-format on

inline auto operator<<(std::ostream& os, const State& state) -> std::ostream&
{
    os << Noelware::Violet::ToString(state);
    return os;
}

struct Address final {
    String Street;
    String City;
    enum State State;
    uint32 PostalCode;

    [[nodiscard]] auto ToString() const noexcept -> String
    {
        std::stringstream buf;
        buf << Street << ' ' << City << ", " << State << ' ' << PostalCode;

        return buf.str();
    }
};

// clang-format off
MARK_SERIALIZABLE_WITH_NAME(
    Address,
    "org.noelware.violet.examples.Address",

    Field("street", &Address::Street),
    Field("city", &Address::City),
    Field("state", &Address::State, { .Default = Some<State>(State::CA) }),
    Field("postal_code", &Address::PostalCode)
);
// clang-format on

struct User final {
    String FirstName;
    String LastName;
    String Username;
    Vec<Address> Addresses;

    [[nodiscard]]
    auto ToString() const noexcept -> String
    {
        std::stringstream buf;
        buf << "User " << FirstName << ' ' << LastName << " (@" << Username << ")\n";

        for (const auto& addr: Addresses) {
            buf << "* " << Noelware::Violet::ToString(addr) << '\n';
        }

        return buf.str();
    }
};

// clang-format off
MARK_SERIALIZABLE_WITH_NAME(
    User,
    "org.noelware.violet.examples.User",

    Field("first_name", &User::FirstName),
    Field("last_name", &User::LastName),
    Field("username", &User::Username),
    Field("addresses", &User::Addresses, {
        .Default = Some<Vec<Address>>()
    })
);
// clang-format on
