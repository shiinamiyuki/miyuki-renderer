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

#ifndef MIYUKI_SERIALIZE_REFLECTION_VISITOR_HPP
#define MIYUKI_SERIALIZE_REFLECTION_VISITOR_HPP

namespace miyuki::refl {
    template <class Visitor> void accept(Visitor visitor, const char **args_s) {}

    template <class Visitor, class T, class... Args>
    void accept(Visitor visitor, const char **args_s, const T &t, const Args &... args) {
        visitor.visit(t, *args_s);
        accept<Visitor, Args...>(visitor, args_s + 1, args...);
    }

    template <class Visitor, class T, class... Args>
    void accept(Visitor visitor, const char **args_s, T &t, Args &... args) {
        visitor.visit(t, *args_s);
        accept<Visitor, Args...>(visitor, args_s + 1, args...);
    }

    template <class Visitor> void accept(Visitor, const char (&args_s)[1]) {}

    template <class Visitor, size_t N, class... Args>
    void accept(Visitor visitor, const char (&args_s)[N], const Args &... args) {
        std::string s = args_s;
        std::array<std::string, sizeof...(Args)> array;
        size_t pos = 0;
        for (size_t i = 0; i < array.size(); i++) {
            while (pos < s.length() && isspace(s[pos])) {
                pos++;
            }
            while (pos < s.length() && s[pos] != ',')
                array[i] += s[pos++];
            pos++;
            while (pos < s.length() && isspace(s[pos])) {
                pos++;
            }
        }
        const char *a[sizeof...(Args)];
        for (size_t i = 0; i < array.size(); i++) {
            a[i] = array[i].c_str();
        }
        accept<Visitor, Args...>(visitor, a, args...);
    }

    template <class Visitor, size_t N, class... Args>
    void accept(Visitor visitor, const char (&args_s)[N], Args &... args) {
        std::string s = args_s;
        std::array<std::string, sizeof...(Args)> array;
        size_t pos = 0;
        for (size_t i = 0; i < array.size(); i++) {
            while (pos < s.length() && isspace(s[pos])) {
                pos++;
            }
            while (pos < s.length() && s[pos] != ',')
                array[i] += s[pos++];
            pos++;
            while (pos < s.length() && isspace(s[pos])) {
                pos++;
            }
        }
        const char *a[sizeof...(Args)];
        for (size_t i = 0; i < array.size(); i++) {
            a[i] = array[i].c_str();
        }
        accept<Visitor, Args...>(visitor, a, args...);
    }
} // namespace miyuki::refl
#define MYK_REFL(Visitor, ...) miyuki::refl::accept(Visitor, #__VA_ARGS__, __VA_ARGS__)

#endif //MIYUKI_SERIALIZE_REFLECTION_VISITOR_HPP
