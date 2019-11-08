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
#pragma once

#include <memory>
#include <type_traits>
#include <vector>

#include <api/defs.h>
#include <api/math.hpp>
#include <api/reflection-visitor.hpp>

namespace miyuki {
    class Entity;
    class PropertyVisitor;
    class Property : public std::enable_shared_from_this<Property> {
      public:
        virtual void accept(PropertyVisitor *visitor) = 0;
    };

    namespace detail {
        template <class T> class BasicProperty {
            std::reference_wrapper<T> ref;
            const char *name;

          public:
            BasicProperty(const char *name, T &ref) : name(name), ref(ref) {}
            const T &getConstRef() const { return ref; }
            T &getRef() { return ref; }
            void set(const T &v) { ref = v; }
            void accept(PropertyVisitor *visitor) { visitor->visit(this); }
        };
    } // namespace detail
    using IntProperty = detail::BasicProperty<int>;
    using FloatProperty = detail::BasicProperty<float>;
    using Vec3fProperty = detail::BasicProperty<Vec3f>;
    using EntityProperty = detail::BasicProperty<std::shared_ptr<Entity>>;
    // using VectorProperty = detail::BasicProperty<std::vector<std::shared_ptr<Entity>>>;
    using FileProperty = detail::BasicProperty<fs::path>;

    class PropertyVisitor {
      public:
        virtual void visit(Property *prop) { prop->accept(this); }
        virtual void visit(IntProperty *) = 0;
        virtual void visit(FloatProperty *) = 0;
        virtual void visit(Vec3fProperty *) = 0;
        virtual void visit(EntityProperty *) = 0;
        virtual void visit(FileProperty *) = 0;
    };

    struct ReflPropertyVisitor {
        PropertyVisitor *visitor;

        void visit(int &v, const char *name) {
            IntProperty prop(name, v);
            prop.accept(visitor);
        }

        void visit(float &v, const char *name) {
            FloatProperty prop(name, v);
            prop.accept(visitor);
        }

        void visit(Vec3f &v, const char *name) {
            Vec3fProperty prop(name, v);
            prop.accept(visitor);
        }

        template <class T>
        std::enable_if_t<std::is_base_of_v<Entity, T>, void> visit(std::shared_ptr<T> &v, const char *name) {
            EntityProperty prop(name, v);
            prop.accept(visitor);
        }
    };
#define MYK_PROP(...)                                                                                                  \
    void acceptPropertyVisitor(miyuki::PropertyVisitor *visitor) override {                                            \
        miyuki::ReflPropertyVisitor refl(visitor);                                                                     \
        MYK_REFL(refl, __VA_ARGS__);                                                                                   \
    }
} // namespace miyuki