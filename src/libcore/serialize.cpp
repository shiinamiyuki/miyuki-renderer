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

#include <api/serialize.hpp>

#include <api/entity.hpp>
#include <unordered_set>
#include <set>
#include <string>
#include <api/log.hpp>

namespace miyuki {
    struct EntityManager {
        static EntityManager *_instance;

        static EntityManager *instance() {
            static std::once_flag flag;
            std::call_once(flag, [&]() { _instance = new EntityManager(); });
            return _instance;
        }

        std::unordered_map<std::string, Type *> types;
        std::unordered_map<std::string, std::set<std::string>> impls;
    };

    EntityManager *EntityManager::_instance;

    void RegisterEntity(const std::string &alias, Type *type) {
        EntityManager::instance()->types[alias] = type;
        log::log("Registered {}\n", alias);
    }

    void BindInterfaceImplementation(const std::string &interface, const std::string &impl) {
        auto &m = EntityManager::instance()->impls;
        if (m.find(interface) == m.end()) {
            m[interface] = {};
        }
        m[interface].insert(impl);
    }

    std::shared_ptr<Entity> CreateEntity(const std::string &type) {
        auto it = EntityManager::instance()->types.find(type);
        if (it != EntityManager::instance()->types.end()) {
            auto ty = it->second;
            auto entity = ty->create();
            return entity;
        }
        return {};
    }

    namespace serialize {
        void WriteEntity(serialize::OutputArchive &ar, const std::shared_ptr<Entity> &e) {
            ar(e);
        }

        std::shared_ptr<Entity> ReadEntity(serialize::InputArchive &ar) {
            std::shared_ptr<Entity> e;
            ar(e);
            return e;
        }
    }
}
