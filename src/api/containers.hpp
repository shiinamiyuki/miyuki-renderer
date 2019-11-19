#pragma once
#include <api/object.hpp>
#include <api/serialize.hpp>
#include <vector>
namespace miyuki {
    template <class T> class Array final : public Object {
        std::vector<std::shared_ptr<Object>> data;

      public:
        MYK_DECL_CLASS(Array, "Array")

        MYK_SER_IMPL(ar) { ar(data); }

        void push_back(const std::shared_ptr<T> &p) { data.push_back(p); }
        template <class Args...> void emplace_back(Args &&... args) { data.emplace_back(std::make_shared<T>(args...)); }

        [[nodiscard]] std::shared_ptr<T> at(size_t index) const { return std::static_pointer_cast<T>(data[index]); }

        void set(size_t index, const std::shared_ptr<T> &p) { data[index] = p; }

        auto size() const { return data.size(); }

        auto begin() { return data.begin(); }
        auto begin() const { return data.begin(); }

        auto end() { return data.end(); }
        auto end() const { return data.end(); }
    };
} // namespace miyuki