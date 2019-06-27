#ifndef MIYUKI_REFLECTION_RUNTIME_HPP
#define MIYUKI_REFLECTION_RUNTIME_HPP
#include "reflection.hpp"
#include "gc.hpp"
namespace Miyuki {
	namespace Reflection {
		class Runtime;

		/*
		Enables serialization/deserialization
		Manages all objects
		*/
		class Runtime : public GC {
			struct Temp {
				UUID id;
				Temp(const UUID& id) :id(id) {}
			};
			struct DeserializationState {
				std::set<Temp*> map;
				std::set<Object*>visited;
				~DeserializationState() {
					for (auto i : map) {
						delete i;
					}
				}
			};			
		
			Result<Object*> _deserialize(const json& j, DeserializationState&state) {
				// null or references are skipped
				if (j.is_null())
					return nullptr;
				if (j.is_string()) {
					auto id = UUIDFromString(j.get<std::string>());
					auto iter = U.objects.find(id);
					if (iter != U.objects.end()) {
						return iter->second;
					}
					// Ugly
					auto t = new Temp(id);
					state.map.insert(t);
					return (Object*)t;
				}
				auto id = UUIDFromString(j.at("id").get<std::string>());
				if (U.objects.find(id) != U.objects.end()) {
					return Error(fmt::format("object with uuid {} already exists", id));
				}
				auto type = j.at("type").get<std::string>();
				auto iter = classInfo.find(type);
				if(iter == classInfo.end())
					return Error(fmt::format("unknown type {}", type));
				auto r = create(iter->second, id);
				if (!r) {
					return r.error();
				}
				auto object = r.value();
				object->deserialize(j, [this,&state](const json& js)->Result<Object*> {
					return _deserialize(js,state);
				});
				return object;
			}

			void resetReferences(Object * object, DeserializationState & state) {
				if (state.visited.find(object) != state.visited.end())
					return;
				state.visited.insert(object);
				for (auto i : object->getReferences()) {
					auto iter = state.map.find((Temp*)i.object);
					auto t = *iter;
					if (iter != state.map.end()) {
						//fmt::print("reset: {}\n", t->id);
						auto o = U.objects[t->id];
						i.reset(o);
						state.visited.insert(o);
					}
					else {
						resetReferences(i.object, state);
					}
				}
			}

			Result<Object*> deserializeR(const json& j) {
				DeserializationState state;
				auto r = _deserialize(j,state);
				if (!r)return r;
				resetReferences(r.value(), state);
				return r.value();
			}

			template<typename T>
			Result<T*> deserializeR(const json& j) {
				auto type = j.at("type").get<std::string>();
				if (type != T::__classinfo__()->name()) {
					return Error(
						fmt::format("Type mismatch: expect {} but have {}\n",
							T::__classinfo__()->name(),
							type));
				}
				Class* info = T::__classinfo__();
				auto name = j.at("id").get<std::string>();
				auto r = deserializeR(j);
				if (r.hasValue()) {
					return (T*)r.value();
				}
				return Error(r.error());
			}
		public:
			template<typename T>
			LocalObject<T> deserialize(const json& j) {
				auto r = deserializeR<T>(j);
				if (r)
					return LocalObject<T>(*this, r.value());
				else {
					throw std::runtime_error(r.error().what());
				}
			}
			LocalObject<Object> deserialize(const json& j) {
				auto r = deserializeR(j);
				if (r)
					return LocalObject<Object>(*this, r.value());
				else {
					throw std::runtime_error(r.error().what());
				}
			}
		};
	}
}

#endif