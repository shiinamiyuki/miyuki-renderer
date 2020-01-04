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

#include <variant>
#include <miyuki.foundation/vectorize.hpp>

#define MYK_DECL_METHOD(Ret, Name, ...) \
template<class T, class... Args> \
struct InvokeMember{             \
    using type = decltype(std::declval<T&>().Name(std::declval<Args>()...));        \
};  \
template<class T, typename = void> \
struct has_##Name : std::false_type { \
}; \
template<class T> \
struct  has_##Name <T, std::void_t<typename InvokeMember<T, __VA_ARGS__>::type>> : std::true_type { \
};

struct Foo {
    MYK_DECL_METHOD(void, f, int)
};

struct Bar {
    void g(int) {}
};

int main() {
//    static_assert(std::is_same_v<std::invoke_result_t<Foo::f, Foo, void>, void>);

}
