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

#include <chrono>
#include <iostream>
#include <string>
#include <thread>

auto main(int argc, char** argv) -> int
{
    if (argc == 0) {
        std::cout << "hello, world!\n";
        return 0;
    }

    if (argc == 1 && std::string(argv[1]) == "sleep") {
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        return 0;
    }

    if (argc == 1 && std::string(argv[1]) == "error") {
        std::cerr << "error!\n";
        return 1;
    }

    if (argc == 1 && std::string(argv[1]) == "stdin") {
        std::string input;
        std::getline(std::cin, input);
        std::cout << "echo:" << input << '\n';

        return 0;
    }

    return 1;
}
