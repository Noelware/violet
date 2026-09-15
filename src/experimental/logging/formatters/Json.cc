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

#include <violet/Experimental/Logging/Formatters/Json.h>
#include <violet/Experimental/Logging/LogRecord.h>

namespace violet::experimental::log {

auto RecordToJson(const Record& record) noexcept -> nlohmann::json
{
    nlohmann::json obj;
    obj["@timestamp"] = record.Timestamp.IntoISO8601();
    obj["message"] = record.Message;

    nlohmann::json log;
    log["logger"] = record.Logger;
    log["level"] = ToString(record.Level);

    obj["log"] = log;

    nlohmann::json src;
    if (!record.Location.File.empty()) {
        src["file"] = record.Location.File;
    }

    if (record.Location.Line > 0) {
        src["line"] = record.Location.Line;
    }

    if (record.Location.Column > 0) {
        src["col"] = record.Location.Column;
    }

    if (!record.Location.Function.empty()) {
        src["function"] = record.Location.Function;
    }

    obj["src"] = src;

    nlohmann::json fields;
    for (const auto& [name, field]: record.Fields) {
        nlohmann::json fieldValue;
        field.Match(
            // clang-format off
            [](Mono)                           -> void {},
            [&fieldValue](const bool& value)   -> void { fieldValue = value; },
            [&fieldValue](const UInt64& value) -> void { fieldValue = value; },
            [&fieldValue](const Int64& value)  -> void { fieldValue = value; },
            [&fieldValue](const String& value) -> void { fieldValue = value; },
            [&fieldValue](const double& value) -> void { fieldValue = value; }
            // clang-format on
        );

        if (!fieldValue.is_null()) {
            fields[name] = fieldValue;
        }
    }

    if (!fields.is_null()) {
        obj["attributes"] = fields;
    }

    return obj;
}

namespace formatter {

auto Json::Format(const Record& record) const -> String
{
    auto value = std::invoke(this->n_config.Transform, record);
    return value.dump(this->n_config.Pretty ? this->n_config.Indentation : -1, ' ', /*ensure_ascii=*/true);
}

} // namespace formatter
} // namespace violet::experimental::log
