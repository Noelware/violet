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

#include <violet/Violet.h>

#if VIOLET_PLATFORM(LINUX)

#include <liburing.h>
#include <violet/Subprocess/PipeReader.h>

#include <unistd.h>

using violet::Err;
using violet::Int32;
using violet::UInt;
using violet::UInt8;
using violet::Vec;
using violet::io::Error;
using violet::subprocess::PipeReader;

using Fd = violet::io::FileDescriptor::value_type;

constexpr unsigned kRingDepth = 8;
constexpr UInt kBufferSize = 65535; // 64KiB per read

namespace {

struct IoUringPipeReader final: public PipeReader {
    VIOLET_IMPLICIT IoUringPipeReader() noexcept
    {
        this->n_buf.reserve(kBufferSize);
    }

    ~IoUringPipeReader() override
    {
        if (this->n_ringOk) {
            ::io_uring_queue_exit(&this->n_ring);
        }
    }

    VIOLET_IMPLICIT_COPY_AND_MOVE(IoUringPipeReader);

    void Register(Fd fd) noexcept override
    {
        this->n_pipeFD = fd;
        this->n_ringOk = ::io_uring_queue_init(kRingDepth, &this->n_ring, /*flags=*/0) == 0;
    }

    [[nodiscard]] auto CaptureAll() const noexcept -> violet::io::Result<Vec<UInt8>> override
    {
        Vec<UInt8> out;
        if (this->n_pipeFD < 0 || !this->n_ringOk) {
            return out;
        }

        while (true) {
            struct io_uring_sqe* sqe = ::io_uring_get_sqe(&this->n_ring);
            ::io_uring_prep_read(
                sqe, this->n_pipeFD, this->n_buf.data(), kBufferSize, /*offset=*/static_cast<violet::UInt64>(-1));

            ::io_uring_sqe_set_flags(sqe, 0);
            if (Int32 ret = ::io_uring_submit(&this->n_ring); ret < 0) {
                return Err(Error::OSError());
            }

            struct io_uring_cqe* cqe = nullptr;
            if (Int32 ret = ::io_uring_wait_cqe(&this->n_ring, &cqe); ret < 0) {
                return Err(Error::OSError());
            }

            Int32 result = cqe->res;
            fprintf(stderr, "cqe->res = %d\n", result);
            ::io_uring_cqe_seen(&this->n_ring, cqe);
            if (result == 0) {
                break;
            }

            if (result < 0) {
                if (-result == EAGAIN || -result == EINTR) {
                    continue;
                }

                return Err(Error::FromOSError(-result));
            }

            out.insert(out.end(), this->n_buf.data(), this->n_buf.data() + static_cast<UInt>(result));
        }

        return out;
    }

    auto WantsNonBlocking() const -> bool override
    {
        return false;
    }

private:
    mutable struct io_uring n_ring{ };
    mutable bool n_ringOk = false;
    mutable Vec<UInt8> n_buf;
    Fd n_pipeFD = -1;
};

} // namespace

auto violet::subprocess::GetPipeReader() noexcept -> UniquePtr<PipeReader>
{
    return std::make_unique<IoUringPipeReader>();
}

#endif
