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

#include <violet/Subprocess/PipeReader.h>

#include <fcntl.h>
#include <sys/epoll.h>
#include <unistd.h>

using violet::Array;
using violet::Err;
using violet::Int32;
using violet::Int64;
using violet::String;
using violet::UInt8;
using violet::Vec;
using violet::io::Error;
using violet::subprocess::PipeReader;

using Fd = violet::io::FileDescriptor::value_type;

namespace {

struct EpollPipeReader final: public PipeReader {
    VIOLET_IMPLICIT EpollPipeReader() noexcept = default;
    ~EpollPipeReader() override
    {
        if (this->n_epollFD >= 0) {
            ::close(this->n_epollFD);
            this->n_epollFD = -1;
        }
    }

    VIOLET_IMPLICIT_COPY_AND_MOVE(EpollPipeReader);

    auto Register(Fd fd) -> violet::io::Result<void> override
    {
        this->n_pipeFD = fd;
        this->n_epollFD = ::epoll_create1(EPOLL_CLOEXEC);
        if (this->n_epollFD < 0) {
            return Err(Error::OSError());
        }

        struct epoll_event event{ };
        event.events = EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLHUP;
        event.data.fd = fd;

        if (::epoll_ctl(this->n_epollFD, EPOLL_CTL_ADD, fd, &event) < 0) {
            return Err(Error::OSError());
        }

        int flags = ::fcntl(fd, F_GETFL, 0);
        if (flags < 0) {
            return Err(Error::OSError());
        }

        if (::fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) {
            return Err(Error::OSError());
        }

        return { };
    }

    [[nodiscard]] auto CaptureAll() const -> violet::io::Result<Vec<UInt8>> override
    {
        Vec<UInt8> output;
        if (this->n_pipeFD < 0 || this->n_epollFD < 0) {
            return Err(VIOLET_IO_ERROR(InvalidData, String,
                "reader was not successfully initialized. did you forget to call `PipeReader::Register`?"));
        }

        Array<UInt8, 4096> chunk;
        while (true) {
            struct epoll_event event{ };
            Int32 bytes = ::epoll_wait(this->n_epollFD, &event, 1, /*__timeout=*/-1);
            if (bytes < 0) {
                if (errno == EINTR) {
                    continue;
                }

                return Err(Error::OSError());
            }

            // Drain all available data whenever `EPOLLIN` fires
            if ((event.events & EPOLLIN) != 0) {
                while (true) {
                    Int64 readBytes = ::read(this->n_pipeFD, chunk.data(), chunk.size());
                    if (readBytes > 0) {
                        output.insert(output.end(), chunk.begin(), chunk.begin() + readBytes);
                    } else if (readBytes == 0) {
                        return output;
                    } else {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            break;
                        }

                        if (errno == EINTR) {
                            continue;
                        }

                        return Err(Error::OSError());
                    }
                }
            }

            // Peer was closed -- drain then exit
            if ((event.events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) != 0) {
                while (true) {
                    Int64 readBytes = ::read(this->n_pipeFD, chunk.data(), chunk.size());
                    if (readBytes <= 0) {
                        break;
                    }

                    output.insert(output.end(), chunk.begin(), chunk.begin() + readBytes);
                }

                return output;
            }
        }
    }

private:
    Fd n_pipeFD = -1;
    Fd n_epollFD = -1;
};

} // namespace

auto violet::subprocess::GetPipeReader() noexcept -> UniquePtr<PipeReader>
{
    return std::make_unique<EpollPipeReader>();
}

#endif
