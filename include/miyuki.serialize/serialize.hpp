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

#ifndef MIYUKI_SERIALIZE_TEST_SERIALIZE_HPP
#define MIYUKI_SERIALIZE_TEST_SERIALIZE_HPP

#include "serialize-fwd.hpp"
#include <type_traits>
#include "json/json.hpp"
#include <stdexcept>
#include <utility>
#include "reflection-visitor.hpp"

namespace miyuki::serialize {
    class NoSuchKeyError : public std::runtime_error {
    public:
        using std::runtime_error::runtime_error;
    };

    class NoSuchTypeError : public std::runtime_error {
    public:
        using std::runtime_error::runtime_error;
    };

    class DowncastError : public std::runtime_error {
    public:
        using std::runtime_error::runtime_error;
    };

    class SerializationError : public std::runtime_error {
    public:
        using std::runtime_error::runtime_error;
    };

    // Base class needed for serializing polymorphic classes
    class Serializable {
    public:
        class Type {
        public:
            [[nodiscard]] virtual const char *name() const = 0;

            virtual Serializable *_create() = 0;

            virtual void save(const Serializable &self, OutputArchive &) = 0;

            virtual void load(Serializable &self, InputArchive &) = 0;

        };


        template<class Archive>
        void save(Archive &) const {}

        template<class Archive>
        void load(Archive &) {}

        [[nodiscard]] virtual Type *getType() const = 0;

        virtual ~Serializable() = default;
    };


    namespace detail {
        template<class T>
        struct check_serializable_static_type {
            struct No {
            };

            template<class U>
            static auto test(int) -> decltype(U::staticType());

            template<class U>
            static auto test(...) -> No;

            static const bool value = std::is_same_v<Serializable::Type *, decltype(test<T>(0))>;
        };

        template<class T, typename = void>
        struct has_member_save : std::false_type {
        };

        template<class T>
        struct has_member_save<T, std::void_t<decltype(std::declval<const T &>().save(
                std::declval<OutputArchive &>()))>> : std::true_type {
        };
        template<class T, typename = void>
        struct has_member_load : std::false_type {
        };

        template<class T>
        struct has_member_load<T, std::void_t<decltype(std::declval<T &>().load(
                std::declval<InputArchive &>()))>> : std::true_type {
        };

        template<class T>
        struct is_json_seriailzable_to_json {
            struct No {
            };

            template<class U>
            static auto test(int) -> decltype(to_json(std::declval<json::json &>(), std::declval<const U &>()));

            template<class U>
            static auto test(...) -> No;

            static const bool value = !std::is_same_v<No, decltype(test<T>(0))>;
        };

        template<class T>
        struct is_json_seriailzable_adl_to_json {
            struct No {
            };

            template<class U>
            static auto test(int) -> decltype(nlohmann::adl_serializer<U>::to_json(std::declval<json::json &>(),
                                                                                   std::declval<const U &>()));

            template<class U>
            static auto test(...) -> No;

            static const bool value = !std::is_same_v<No, decltype(test<T>(0))>;
        };

        template<class T>
        struct is_json_seriailzable {
            static const bool value =
                    is_json_seriailzable_adl_to_json<T>::value || is_json_seriailzable_to_json<T>::value;
        };


        template<class T>
        struct is_json_deseriailzable {
            struct No {
            };

            template<class U>
            static auto test(int) -> decltype(std::declval<json::json>().get<U>());

            template<class U>
            static auto test(...) -> No;

            static const bool value = !std::is_same_v<No, decltype(test<T>(0))>;
        };

    }

    class Context {
        std::unordered_map<std::string, Serializable::Type *> types;
    public:
        template<class T>
        void registerType() {
            static_assert(std::is_base_of_v<Serializable, T>, "T must implement Serializable");
            static_assert(detail::check_serializable_static_type<T>::value, "T must have static Type * staticType()");
            types[T::staticType()->name()] = T::staticType();
        }

        Serializable::Type *getType(const std::string &s) {
            auto it = types.find(s);
            if (it == types.end()) {
                throw NoSuchTypeError(std::string("No such type named ").append(s));
            }
            return it->second;
        }
    };

    template<class T>
    struct NVP {
        NVP(const char *name, const T &ref) : name(name), ref(ref) {}

        const char *name;
        const T &ref;
    };

    template<class T>
    struct NVP_NC {
        NVP_NC(const char *name, T &ref) : name(name), ref(ref) {}

        const char *name;
        T &ref;
    };

    template<class T>
    struct is_nvp {
        static const bool value = false;
    };
    template<class T>
    struct is_nvp<NVP<T>> {
        static const bool value = true;
    };

    template<class T>
    struct is_nvp<NVP_NC<T>> {
        static const bool value = true;
    };

    template<class T>
    NVP<T> make_nvp(const char *name, const T &arg) {
        return NVP<T>(name, arg);
    }

    template<class T>
    NVP_NC<T> make_nvp(const char *name, T &arg) {
        return NVP_NC<T>(name, arg);
    }

    struct ArchiveBase {
        std::vector<std::string> locator;

        template<class F, class... Args>
        void tryInvoke(F &&f, Args &&... args) {
            try {
                f(std::forward<Args>(args)...);
            }
            catch (serialize::SerializationError &e) {
                throw e;
            }
            catch (std::exception &e) {
                std::string msg;
                for (auto &i:locator) {
                    msg.append(i);
                }
                throw SerializationError(std::string("error: ").append(e.what()).append(" in ").append(msg));
            }
        }
    };

    class OutputArchive : public ArchiveBase {
        Context &context;
        std::vector<std::reference_wrapper<json::json>> stack;
        std::unordered_map<Serializable *, std::reference_wrapper<json::json>> ptrs;
        std::vector<int> counter;
        json::json data;

    public:

        [[nodiscard]] json::json getData() const {
            return data;
        }

        json::json &_top() {
            return stack.back();
        }

        void _makeNode(json::json &ref) {
            stack.emplace_back(ref);
            counter.emplace_back(0);
        }

        void _popNode() {
            stack.pop_back();
            counter.pop_back();
        }

        explicit OutputArchive(Context &context) : context(context) {
            _makeNode(data);
        }

        template<class T>
        void _save_nvp(const char *name, const T &value) {
            _top()[name] = json::json();
            _makeNode(_top()[name]);
            locator.emplace_back(std::string("/").append(name));
            _save(value);
            locator.pop_back();
            _popNode();
        }

        template<class T>
        void _save_nvp(const std::string &name, const T &value) {
            _top()[name] = json::json();
            _makeNode(_top()[name]);
            locator.emplace_back(std::string("/").append(name));
            _save(value);
            locator.pop_back();
            _popNode();
        }

        template<class T>
        void _save(NVP<T> &&nvp) {
            _save_nvp(nvp.name, nvp.ref);
        }

        template<class T>
        std::enable_if_t<detail::is_json_seriailzable<T>::value, void> _save(const T &arg) {
            _top() = arg;
        }

        template<class T>
        std::enable_if_t<detail::has_member_save<T>::value, void> _save(const T &arg) {
            arg.save(*this);
        }

        template<class T>
        std::enable_if_t<std::is_base_of_v<Serializable, T> && detail::has_member_save<T>::value, void>
        _save(const std::shared_ptr<T> &ptr) {
            //static_assert(detail::check_serializable_static_type<T>::value, "T must have static Type * staticType()");
            if (!ptr)return;
            auto *raw = static_cast<Serializable *>(ptr.get());
            auto it = ptrs.find(raw);
            if (it == ptrs.end()) {
                _top() = json::json{
                        {"type", ptr->getType()->name()}
                };
                _top()["props"] = json::json();
                _makeNode(_top()["props"]);
                ptr->getType()->save(*ptr, *this);
                _popNode();
                ptrs.emplace(raw, _top());
            } else {
                auto addr = reinterpret_cast<size_t>(raw);
                ptrs.at(raw).get()["addr"] = std::to_string(addr);
                _top() = json::json{
                        {"addr", std::to_string(addr)},
                        {"type", ptr->getType()->name()}
                };
            }
        }

        template<class T>
        std::enable_if_t<std::is_base_of_v<Serializable, T> && detail::has_member_save<T>::value, void>
        _save(const std::vector<std::shared_ptr<T>> &vec) {
            _top() = json::json::array();
            auto &arr = _top();
            auto cnt = 0;
            for (const auto &i :vec) {
                arr.emplace_back();
                _makeNode(arr.back());
                locator.emplace_back(std::string("/").append(std::to_string(cnt)));
                _save(i);
                locator.pop_back();
                _popNode();
                cnt++;
            }
        }

        template<class T>
        std::enable_if_t<std::is_base_of_v<Serializable, T>, void>
        _save(const std::unordered_map<std::string, std::shared_ptr<T>> &map) {
            static_assert(detail::check_serializable_static_type<T>::value, "T must have static Type * staticType()");
            _top() = json::json::object();
            auto &dict = _top();
            for (const auto &i :map) {
                dict[i.first] = json::json();
                _makeNode(dict[i.first]);
                locator.emplace_back(std::string("/").append(i.first));
                _save(i.second);
                locator.pop_back();
                _popNode();
            }
        }

        template<class T>
        void _save(const NVP<T> &nvp) {
            _save_nvp(nvp.name, nvp.ref);
        }

        template<class T>
        void _save2(T &&arg) {
            if constexpr (is_nvp<T>::value) {
                _save_nvp(arg);
            } else {
                _save_nvp(std::string("$val").append(std::to_string(counter.back())), std::forward<T>(arg));
                counter.back()++;
            }
        }

        template<class T>
        void _save_v(T &&arg) {
            _save2(arg);
        }

        template<class T, class... Args>
        void _save_v(T &&head, Args &&...args) {
            _save2(head);
            _save_v(args...);
        }

        template<class... Args>
        void operator()(Args &&...args) {
            _save_v(args...);
        }

    };

    class InputArchive : public ArchiveBase {
        Context &context;
        const json::json &data;
        std::vector<std::reference_wrapper<const json::json>> stack;
        std::unordered_map<size_t, std::shared_ptr<Serializable>> ptrs;
        std::vector<int> counter;

    public:

        const json::json &_top() { return stack.back(); }

        void _makeNode(const json::json &ref) {
            stack.emplace_back(ref);
            counter.emplace_back(0);
        }

        void _popNode() {
            stack.pop_back();
            counter.pop_back();
        }

        explicit InputArchive(Context &context, const json::json &data) : context(context), data(data) {
            stack.emplace_back(data);
        }


        template<class T>
        std::enable_if_t<detail::is_json_deseriailzable<T>::value, void> _load(T &arg) {
            arg = _top().get<T>();
        }

        template<class T>
        std::enable_if_t<detail::has_member_load<T>::value, void> _load(T &arg) {
            arg.load(*this);
        }

        template<class T>
        std::enable_if_t<std::is_base_of_v<Serializable, T>, void>
        _load(std::shared_ptr<T> &ptr) {
            if (_top().is_null()) {
                ptr = nullptr;
                return;
            }
            auto type_s = _top().at("type").get<std::string>();
            auto type = context.getType(type_s);
            if (_top().contains("props")) {
                if (_top().contains("addr")) {
                    auto addr = std::stol(_top().at("addr").get<std::string>());
                    auto it = ptrs.find(addr);
                    if (it == ptrs.end()) {
                        ptr = std::shared_ptr<T>(dynamic_cast<T *>(type->_create()));
                        ptrs[addr] = ptr;
                    } else {
                        ptr = std::dynamic_pointer_cast<T>(it->second);
                    }
                } else {
                    ptr = std::shared_ptr<T>(dynamic_cast<T *>(type->_create()));
                }
                _makeNode(_top().at("props"));
                type->load(*ptr, *this);
                _popNode();
            } else {
                auto addr = std::stol(_top().at("addr").get<std::string>());
                ptr = std::dynamic_pointer_cast<T>(ptrs.at(addr));
            }
        }

        template<class T>
        std::enable_if_t<std::is_base_of_v<Serializable, T>, void>
        _load(std::vector<std::shared_ptr<T>> &vec) {
            for (int i = 0; i < _top().size(); i++) {
                _makeNode(_top().at(i));
                locator.emplace_back(std::string("/").append(std::to_string(i)));
                vec.emplace_back();
                _load(vec.back());
                locator.pop_back();
                _popNode();
            }
        }

        template<class T>
        std::enable_if_t<std::is_base_of_v<Serializable, T>, void>
        _load(std::unordered_map<std::string, std::shared_ptr<T>> &map) {
            for (auto &el : _top().items()) {
                _makeNode(el.value());
                locator.emplace_back(std::string("/").append(el.key()));
                map[el.key()] = nullptr;
                _load(map[el.key()]);
                locator.pop_back();
                _popNode();
            }
        }

        template<class T>
        void _load_nvp(const char *name, T &value) {
            if (_top().contains(name)) {
                _makeNode(_top()[name]);
                locator.emplace_back(std::string("/").append(name));
                _load(value);
                locator.pop_back();
                _popNode();
            }
        }

        template<class T>
        void _load_nvp(const std::string &name, T &value) {
            if (_top().contains(name)) {
                _makeNode(_top()[name]);
                locator.emplace_back(std::string("/").append(name));
                _load(value);
                locator.pop_back();
                _popNode();
            }
        }

        template<class T>
        void _load(NVP_NC<T> &&nvp) {
            _load_nvp(nvp.name, nvp.ref);
        }

        template<class T>
        void _load2(T &&arg) {
            if constexpr (is_nvp<T>::value) {
                _load_nvp(arg);
            } else {
                _load_nvp(std::string("$val").append(std::to_string(counter.back())), std::forward<T>(arg));
                counter.back()++;
            }
        }

        template<class T>
        void _load_v(T &&arg) {
            _load2(arg);
        }

        template<class T, class... Args>
        void _load_v(T &&head, Args &&...args) {
            _load2(head);
            _load_v(args...);
        }

        template<class... Args>
        void operator()(Args &&...args) {
            _load_v(args...);
        }
    };

    struct AutoSaveVisitor {
        OutputArchive &ar;

        template<class T>
        void visit(T &&arg, const char *name) {
            ar._save_nvp(name, arg);
        }
    };

    struct AutoLoadVisitor {
        InputArchive &ar;

        template<class T>
        void visit(T &&arg, const char *name) {
            ar._load_nvp(name, arg);
        }
    };

    template<class T>
    json::json toJson(Context &context, const T &data) {
        OutputArchive ar(context);
        ar.tryInvoke([&]() {
            ar._save(data);
        });
        return ar.getData();
    }

    template<class T>
    T fromJson(Context &context, const json::json &j) {
        InputArchive ar(context, j);
        T data;
        ar.tryInvoke([&]() {
            ar._load(data);
        });
        return data;
    }

#define MYK_SER(...) template<class Archive>void save(Archive & ar)const { miyuki::serialize::AutoSaveVisitor v{ar}; MYK_REFL(v, __VA_ARGS__); }\
                template<class Archive>void load(Archive & ar) { miyuki::serialize::AutoLoadVisitor v{ar}; MYK_REFL(v, __VA_ARGS__); }
#define MYK_TYPE(Class, Alias)                                                                                                                  \
    static miyuki::serialize::Serializable::Type * staticType(){                                                                                \
        class T : public  miyuki::serialize::Serializable::Type{                                                                                \
        public:                                                                                                                                 \
            const char *name() const override {                                                                                                 \
                return Alias;                                                                                                                   \
            }                                                                                                                                   \
            Serializable *_create() override {                                                                                                  \
                return new Class();                                                                                                             \
            }                                                                                                                                   \
            void save(const Serializable &self, miyuki::serialize::OutputArchive &ar)override{static_cast<const Class&>(self).save(ar);}                           \
            void load(Serializable &self, miyuki::serialize::InputArchive &ar)override{static_cast<Class&>(self).load(ar);}                                        \
        };                                                                                                                                      \
        static T t;                                                                                                                             \
        return &t;                                                                                                                              \
    }                                                                                                                                           \
    miyuki::serialize::Serializable::Type *  getType()const override{return staticType();}
}


#endif //MIYUKI_SERIALIZE_TEST_SERIALIZE_HPP
