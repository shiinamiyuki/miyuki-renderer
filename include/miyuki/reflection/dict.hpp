#pragma once

#include "object.hpp"
#include "primitive.hpp"

namespace Miyuki {
	namespace Reflection {
		// Only supports std::string -> T right now
		template<class T>
		class Dict final : public Object {
			using Ty = typename _ConvertToPrimitiveType<T>::type;
			std::unordered_map<std::string, Ty*> _map;
			using MapT = decltype(_map);
		public:
			static Class* __classinfo__() {
				static Class* info = nullptr;
				static std::once_flag flag;
				std::call_once(flag, [&]() {
					info = new Class();
					auto s = new std::string("Miyuki::Reflection::Array<");
					*s += Ty::__classinfo__()->name();
					*s += ">";
					info->_name = s->c_str();
					o->classInfo.size = sizeof(Dict<T>);
					info->classInfo.base = Object::__classinfo__();
					info->classInfo.ctor = [=](const UUID& n) {return new Dict<T>(n); };
				});
				return info;
			}
			Dict(const UUID& id) :Object(__classinfo__(), id) {}
			void insert(const std::string& key, Ty* value) {
				_map[key] = value;
			}
			void insert(const std::string& key, Object* value) {
				_map[key] = StaticCast<Ty>(value);
			}
			const Ty* at(const std::string& key)const {
				return _map.at(key);
			}
			Ty*& at(const std::string& key) {
				return _map.at(key);
			}
			auto find(const std::string& key)const {
				return _map.find(key);
			}
			auto erase(MapT::const_iterator  it) {
				return _map.erase(it);
			}
			void serialize(json& j, SerializationState& state)const override {
				Object::serialize(j, state);
				j["dict"] = json::object();
				for (const auto& i : _map) {
					json tmp;
					auto value = i->second;
					value->serialize(tmp, state);
					j["dict"][i->first] = tmp;
				}
			}
			virtual std::vector<Object::Reference> getReferences()override {
				auto result = Object::getReferences();

				for (auto i : _map) {
					auto key = i->first;
					result.emplace_back([=](Object* o)->void {
						_map.at(key) = StaticCast<Ty>(o); },
						i->second);
				}
				return result;
			}
			virtual void deserialize(const json& j, const Resolver& resolve)override {
				Object::deserialize(j, resolve);
				for (const auto& i : j["dict"].items()) {
					if (auto r = resolve(i.value()))
						(*this)[i.key().get<std::string>()] = (StaticCast<Ty>(r.value()));
				}
			}
		};
	}
}