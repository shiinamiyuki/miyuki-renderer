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

#ifndef MIYUKIRENDERER_SERIALIZE_HPP
#define MIYUKIRENDERER_SERIALIZE_HPP

#include <cereal/cereal.hpp>
#include <cereal/archives/json.hpp>
#include <nlohmann/json.hpp>
#include <unordered_set>
#include <api/defs.h>
#include <api/detail/entity-funcs.h>
#include <cereal/details/static_object.hpp>

namespace miyuki {
    class Entity;
}
namespace miyuki::serialize {
    using nlohmann::json;

    class InputArchive;

    class OutputArchive;

    class InputArchive : public cereal::JSONInputArchive {
        std::unordered_map<size_t, std::shared_ptr<Entity>> _refs;
        std::unordered_set<size_t> _serialized;
    public:
        using JSONInputArchive::JSONInputArchive;

        template<class T>
        void addRef(size_t addr, const std::shared_ptr<T> &p) {
            _refs[addr] = p;
        }

        template<class T>
        void addRef(size_t addr, const std::weak_ptr<T> &p) {
            _refs[addr] = p.lock();
        }

        void addSerializedRef(size_t addr) {
            _serialized.insert(addr);
        }

        bool hasRegistered(size_t addr) const {
            return _refs.find(addr) != _refs.end();
        }

        bool hasSerialized(size_t addr) const {
            return _serialized.find(addr) != _serialized.end();
        }

        template<class T>
        std::shared_ptr<T> getRef(size_t addr) const {
            return std::dynamic_pointer_cast<T>(_refs.at(addr));
        }
    };

    class OutputArchive : public cereal::JSONOutputArchive {
        std::unordered_set<Entity *> _serialized;
    public:
        using JSONOutputArchive::JSONOutputArchive;

        template<class T>
        void addSerialized(std::shared_ptr<T> p) {
            _serialized.insert(p.get());
        }


        template<class T>
        bool hasSerialized(std::shared_ptr<T> p) const {
            return _serialized.find(p.get()) != _serialized.end();
        }

        template<class T>
        void addSerialized(std::weak_ptr<T> p) {
            _serialized.insert(p.get());
        }


        template<class T>
        bool hasSerialized(std::weak_ptr<T> p) const {
            return _serialized.find(p.get()) != _serialized.end();
        }

    };
}

namespace cereal {
    template<class Archive, class T>
    inline void
    save(Archive &ar, std::shared_ptr<T> const &p) {
        //using Archive = kaede::OutputArchive;
        if (!p) {
            ar(CEREAL_NVP_("#valid", 0));
        } else {

            ar(CEREAL_NVP_("#type", std::string(p->getType()->name())));
            ar(CEREAL_NVP_("#addr", reinterpret_cast<size_t>(static_cast<miyuki::Entity *>(p.get()))));
            auto _archive = dynamic_cast<miyuki::serialize::OutputArchive *>(&ar);
            if (!_archive) {
                MIYUKI_THROW(std::runtime_error, "Archive must be of OutputArchive");
            }
            auto &archive = *_archive;
            if (!archive.hasSerialized(p)) {
                archive.addSerialized(p);
                p->save(archive);
                ar(CEREAL_NVP_("#valid", 2));
            } else {
                ar(CEREAL_NVP_("#valid", 1));
            }
        }
    }

    template<class Archive, class T>
    inline void
    load(Archive &ar, std::shared_ptr<T> &p) {
        int32_t valid;
        ar(CEREAL_NVP_("#valid", valid));
        if (!valid) {
            p = nullptr;
        } else {
            size_t addr;
            ar(CEREAL_NVP_("#addr", addr));
            std::string type;
            ar(CEREAL_NVP_("#type", type));
            auto _archive = dynamic_cast<miyuki::serialize::InputArchive *>(&ar);
            if (!_archive) {
                MIYUKI_THROW(std::runtime_error, "Archive must be of InputArchive");
            }
            auto &archive = *_archive;
            if (!archive.hasRegistered(addr)) {
                p = std::dynamic_pointer_cast<T>(miyuki::CreateEntity(type));
                archive.addRef(addr, p);
            } else {
                p = archive.getRef<T>(addr);
            }
            if (valid & 2) {
                if (archive.hasSerialized(addr)) {
                    MIYUKI_THROW(std::runtime_error, "Load the same object twice!");
                }
                p->load(archive);
                archive.addSerializedRef(addr);
            }
        }
    }

    template<class Archive, class T, typename = std::enable_if_t<std::is_base_of_v<miyuki::Entity, T>>>
    inline void
    save(Archive &ar, std::weak_ptr<T> const &p) {
        //using Archive = kaede::OutputArchive;
        if (!p) {
            ar(CEREAL_NVP_("#valid", 0));
        } else {

            ar(CEREAL_NVP_("#type", std::string(p->getType()->name())));
            ar(CEREAL_NVP_("#addr", reinterpret_cast<size_t>(static_cast<miyuki::Entity *>(p.get()))));
            auto _archive = dynamic_cast<miyuki::serialize::OutputArchive *>(&ar);
            if (!_archive) {
                MIYUKI_THROW(std::runtime_error, "Archive must be of OutputArchive");
            }
            auto &archive = *_archive;
            if (!archive.hasSerialized(p)) {
                archive.addSerialized(p);
                p->save(archive);
                ar(CEREAL_NVP_("#valid", 2));
            } else {
                ar(CEREAL_NVP_("#valid", 1));
            }
        }
    }

    template<class Archive, class T>
    inline void
    load(Archive &ar, std::weak_ptr<T> &p) {
        int32_t valid;
        ar(CEREAL_NVP_("#valid", valid));
        if (!valid) {
            p = nullptr;
        } else {
            size_t addr;
            ar(CEREAL_NVP_("#addr", addr));
            std::string type;
            ar(CEREAL_NVP_("#type", type));
            auto _archive = dynamic_cast<miyuki::serialize::InputArchive *>(&ar);
            if (!_archive) {
                MIYUKI_THROW(std::runtime_error, "Archive must be of InputArchive");
            }
            auto &archive = *_archive;
            if (!archive.hasRegistered(addr)) {
                auto ref = std::dynamic_pointer_cast<T>(miyuki::CreateEntity(type));
                archive.addRef(addr, ref);
                p = ref;
            } else {
                p = archive.getRef<T>(addr);
            }
            if (valid & 2) {
                if (archive.hasSerialized(addr)) {
                    MIYUKI_THROW(std::runtime_error, "Load the same object twice!");
                }
                p->load(archive);
                archive.addSerializedRef(addr);
            }
        }
    }
}
namespace cereal {
    template<class Archive, class T>
    void safe_apply(Archive &ar, const char *name, T &val) {
        try {
            ar(CEREAL_NVP_(name, val));
        } catch (cereal::Exception &e) {
            //
        }
    }
}

namespace miyuki::serialize {
    template<class Archive>
    struct ArchivingVisitor {
        Archive &ar;

        ArchivingVisitor(Archive &ar) : ar(ar) {}

        template<class T>
        void visit(T &v, const char *name) {
            cereal::safe_apply(ar, name, v);
        }

        template<class T>
        void visit(const T &v, const char *name) {
            cereal::safe_apply(ar, name, v);
        }
    };

    template<class Visitor, class T>
    std::enable_if_t<std::is_class_v<T>, void> accept(Visitor v, T t) {
        t.accept(v);
    }

    template<class Visitor>
    void _accept(Visitor visitor, const char **args_s) {}

    template<class Visitor, class T, class... Args>
    void _accept(Visitor visitor, const char **args_s, const T &t, const Args &... args) {
        visitor.visit(t, *args_s);
        _accept<Visitor, Args...>(visitor, args_s + 1, args...);
    }


    template<class Visitor, class T, class... Args>
    void _accept(Visitor visitor, const char **args_s, T &t, Args &... args) {
        visitor.visit(t, *args_s);
        _accept<Visitor, Args...>(visitor, args_s + 1, args...);
    }

    template<class Visitor>
    void _accept(Visitor, const char (&args_s)[1]) {}

    template<class Visitor, size_t N, class... Args>
    void _accept(Visitor visitor, const char (&args_s)[N], const Args &... args) {
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
        _accept<Visitor, Args...>(visitor, a, args...);
    }

    template<class Visitor, size_t N, class... Args>
    void _accept(Visitor visitor, const char (&args_s)[N], Args &... args) {
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
        _accept<Visitor, Args...>(visitor, a, args...);


    }

    template<class... Args>
    void _assign(Args...) noexcept {}

    template<class>
    struct GetMethodSelfType {
    };
    template<class T, class Ret, class... Args>
    struct GetMethodSelfType<Ret (T::*)(Args...) const> {
        using type = std::decay_t<T>;
    };
}


#define MYK_DECL_CLASS(Classname, Alias, ...) \
    using Self = Classname;\
    static miyuki::Type * staticType(){\
        return miyuki::GetStaticType<Self>(Alias);\
    }\
    miyuki::Type * getType()const override{return staticType();}\
    static void _register(){\
        static_assert(std::is_final_v<Self>, Alias " must be final");\
        miyuki::RegisterEntity(Alias, staticType());\
        std::string interface; \
        miyuki::serialize::_assign(__VA_ARGS__);\
        if(!interface.empty())  {\
            miyuki::BindInterfaceImplementation(interface, Alias);\
        }\
    }


#define _MYK_REFL(...)     \
    template<class Visitor>\
    void accept(Visitor visitor){\
       miyuki::serialize::_accept(visitor, #__VA_ARGS__ , __VA_ARGS__);\
    } \
    template<class Visitor>\
    void accept(Visitor visitor)const {\
       miyuki::serialize::_accept(visitor, #__VA_ARGS__ , __VA_ARGS__);\
    }
#define _MYK_POLY_SER    \
     void save(miyuki::serialize::OutputArchive& ar)const override{\
        using Archive = miyuki::serialize::OutputArchive;ar(CEREAL_NVP_("#data", *this));\
    }  \
    void load(miyuki::serialize::InputArchive& ar)override{  \
        using Archive = miyuki::serialize::InputArchive;ar(CEREAL_NVP_("#data", *this));\
    }  \
    bool isSerializable()const override{return true;}

#define MYK_SER_IMPL(ar)  \
    template<class Archive> \
    void save(Archive & ar)const{const_cast<std::decay_t<decltype(*this)>&>(*this)._save_load(ar);}  \
    template<class Archive> \
    void load(Archive & ar){_save_load(ar);}  \
    _MYK_POLY_SER \
    template<class Archive>  \
    void _save_load(Archive& ar)

#define MYK_AUTO_SER(...)\
    _MYK_REFL(__VA_ARGS__)\
    template<class Archive>      \
    void save(Archive & ar)const{     \
        miyuki::serialize::ArchivingVisitor v(ar);     \
        accept(v);\
    } \
    template<class Archive>      \
    void load(Archive & ar){     \
         miyuki::serialize::ArchivingVisitor v(ar);     \
        accept(v);\
    } \
    _MYK_POLY_SER

#include <api/math.hpp>
namespace miyuki {
    using json = nlohmann::json;

    template<class T, size_t N>
    inline void to_json(json &j, const Vec<T, N> &v) {
        j = json::array();
        for (int i = 0; i < N; i++) {
            j[i] = v[i];
        }
    }

    template<class T, size_t N>
    inline void from_json(const json &j, Vec <T, N> &v) {
        for (int i = 0; i < N; i++) {
            v[i] = j[i];
        }
    }

    inline void to_json(json &j, const Vec3f &v) {
        j = json::array();
        for (int i = 0; i < 3; i++) {
            j[i] = v[i];
        }
    }


    inline void from_json(const json &j, Vec3f &v) {
        for (int i = 0; i < 3; i++) {
            v[i] = j[i];
        }
    }

    inline void to_json(json &j, const Matrix4 &m) {
        j = json::array();
        for (int i = 0; i < 4; i++) {
            j[i] = m[i];
        }
    }


    inline void from_json(const json &j, Matrix4 &m) {
        for (int i = 0; i < 4; i++) {
            m[i] = j[i].get < Vec < Float, 4 >> ();
        }
    }

    inline void to_json(json &j, const Transform &transform) {
        j = transform.matrix();
    }


    inline void from_json(const json &j, Transform &transform) {
        transform = Transform(j.get<Matrix4>());
    }
}


#endif //MIYUKIRENDERER_SERIALIZE_HPP
