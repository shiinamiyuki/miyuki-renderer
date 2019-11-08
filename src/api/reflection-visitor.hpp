#pragma once

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
}