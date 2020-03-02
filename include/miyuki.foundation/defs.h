// MIT License
// 
// Copyright (c) 2019 椎名深雪
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

#ifndef MIYUKIRENDERER_DEFS_H
#define MIYUKIRENDERER_DEFS_H

#ifdef MYK_USE_BOOST_FS
#include <boost/filsystem.hpp>
namespace miyuki {
    namespace fs = boost::filesystem;
}
#else

#ifdef __GNUC__
#if __GNUC__   >= 8

#include <filesystem>
namespace miyuki {
    namespace fs = std::filesystem;
}
#else
#include <experimental/filesystem>
namespace miyuki {
    namespace fs = std::experimental::filesystem;
}
#endif

#else

#include <filesystem>
namespace miyuki {
    namespace fs = std::filesystem;
}
#endif
#endif

namespace miyuki {
    using Float = float;
    constexpr Float Pi = 3.1415926535f;
    constexpr Float Pi2 = Pi * 0.5f;
    constexpr Float Pi4 = Pi * 0.25f;
    constexpr Float InvPi = 1.0f / Pi;
    constexpr Float Inv4Pi = 1.0f / (4.0f * Pi);
    constexpr Float MaxFloat = std::numeric_limits<Float>::max();
    constexpr Float MinFloat = std::numeric_limits<Float>::lowest();
    constexpr Float MachineEpsilon = std::numeric_limits<Float>::epsilon();

    constexpr Float gamma(int n) {
        return n * MachineEpsilon / (1 - n * MachineEpsilon);
    }

    template<class T>
    inline T RadiansToDegrees(T x) {
        return x * InvPi * 180.0f;
    }

    template<class T>
    inline T DegreesToRadians(T x) {
        return x / 180.0f * Pi;
    }

    struct CurrentPathGuard {
        CurrentPathGuard() : current(fs::current_path()) {}

        ~CurrentPathGuard() {
            fs::current_path(current);
        }

    private:
        fs::path current;
    };

#define MIYUKI_NOT_IMPLEMENTED() std::abort()
#define MIYUKI_THROW(exception, ...) throw exception(__VA_ARGS__)

#ifndef MYK_RELEASE
#define MIYUKI_CHECK(expr)   do{if(!(expr)){ fmt::print(stderr, "{} failed at {}:{}\n",#expr, __FILE__,__LINE__);   }}while(0)

    inline void __assert(bool expr, const std::string &msg) {
        if (!expr) {
            MIYUKI_THROW(std::runtime_error, msg.c_str());
        }
    }

#define MIYUKI_ASSERT(expr)   __assert(expr, fmt::format("{} failed at {}:{}\n",#expr, __FILE__,__LINE__))

#else
#define MIYUKI_CHECK(expr)
#define MIYUKI_ASSERT(expr)
#endif
#ifdef _MSC_VER

#define     MYK_PUBLIC_API __declspec(dllexport)
#elif defined(__GNUC__) || defined(__MINGW32__)
#define     MYK_PUBLIC_API  __attribute__ ((visibility("default"))) __declspec(dllexport)
#define     MYK_PRIVATE_API  __attribute__ ((visibility("hidden")))
#endif
}
#endif //MIYUKIRENDERER_DEFS_H
