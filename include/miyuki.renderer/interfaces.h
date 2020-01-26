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

#ifndef MIYUKI_FOUNDATION_INTERFACES_H
#define MIYUKI_FOUNDATION_INTERFACES_H

#include <miyuki.serialize/serialize.hpp>

namespace miyuki {
    template<class... Args>
    void _assign(Args... args) {}

#define MYK_INTERFACE(Classname, Alias)
#define MYK_DECL_CLASS(Classname, Alias, ...) MYK_TYPE(Classname, Alias) \
    static std::string _interface(){\
        std::string interface="";\
        _assign(__VA_ARGS__);\
        return interface;\
    }

    class SerializeContext : public serialize::Context {
        std::unordered_map<std::string, std::vector<std::string>> _impls;
    public:
        template<class F>
        void foreachImpl(const std::string &interface, F &&f) {
            for (auto &i: _impls.at(interface)) {
                f(i);
            }
        }

        template<class T>
        void registerType() {
            serialize::Context::registerType<T>();
            _impls[T::_interface()].push_back(T::staticType()->name());
        }
    };

}
#endif //MIYUKI_FOUNDATION_INTERFACES_H
