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

#include <violet/Experimental/Logging/AsyncSink.h>
#include <violet/Experimental/Logging/Logger.h>
#include <violet/Experimental/Logging/Sink.h>

namespace violet::experimental::log {

Entry::~Entry()
{
    if (this->n_emit && this->n_parent != nullptr) {
        for (auto& sink: this->n_parent->n_sinks) {
            sink->Emit(this->n_record);
        }

        for (auto& sink: this->n_parent->n_asyncSinks) {
            sink->Enqueue(this->n_record);
        }

        this->n_emit = false;
    }
}

Logger::~Logger()
{
    this->flush();
}

void Logger::flush()
{
    for (auto sink: this->n_sinks) {
        sink->Flush();
    }

    for (auto sink: this->n_asyncSinks) {
        sink->Flush();
    }

    this->n_sinks.clear();
    this->n_asyncSinks.clear();
}

} // namespace violet::experimental::log
