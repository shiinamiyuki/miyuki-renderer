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

#ifndef MIYUKIRENDERER_ENTITY_HPP
#define MIYUKIRENDERER_ENTITY_HPP

#include <api/defs.h>
#include <memory>
#include <unordered_map>
#include <variant>
#include <api/entity.hpp>
#include <mutex>

#include <nlohmann/json_fwd.hpp>

namespace miyuki {
    using json = nlohmann::json;
    namespace serialize {
        class InputArchive;

        class OutputArchive;
    }
    class Type;

    // Base class for all entities in rendering
    class Entity {
    public:
        [[nodiscard]] virtual Type *getType() const = 0;

        virtual void save(serialize::OutputArchive &) const {}

        virtual void load(serialize::InputArchive &) {}

        virtual void initialize(const json &) const {}

        virtual bool isSerializable() const { return false; }
    };


    class Type {
    public:
        virtual std::shared_ptr<Entity> create() const = 0;

        virtual const char *name() const = 0;

    };


    template<class T>
    Type *GetStaticType(const char *_name) {
        struct TypeImpl : Type {
            const char *_name;

            explicit TypeImpl(const char *name) : _name(name) {}

            [[nodiscard]] std::shared_ptr<Entity> create() const override {
                return std::make_shared<T>();
            }

            [[nodiscard]] const char *name() const override {
                return _name;
            }
        };
        static std::once_flag flag;
        static TypeImpl *type = nullptr;
        std::call_once(flag, [&]() { type = new TypeImpl(_name); });
        return type;
    }


    enum class NodeType {
        ENone,
        EInt,
        EBool,
        EInt3,
        EFloat3,
        EColorHSV,
        EColorRGB,
        EImageTexture,
        EEntity
    };

    struct Connection {
        NodeType type = NodeType::ENone;
        void *data = nullptr;
    };

    class ConnectableEntity : public Entity {
    public:

    };
}

#endif //MIYUKIRENDERER_ENTITY_HPP
