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
				std::string name;
				Temp(const std::string& name) :name(name) {}
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
		public:
			Result<Object*> _deserialize(const json& j, DeserializationState&state) {
				// null or references are skipped
				if (j.is_null())
					return nullptr;
				if (j.is_string()) {
					auto name = j.get<std::string>();
					auto iter = U.named.find(name);
					if (iter != U.named.end()) {
						return iter->second;
					}
					fmt::print("name={}\n",name);
					// Ugly
					auto t = new Temp(name);
					state.map.insert(t);
					return (Object*)t;
				}
				auto name = j.at("name").get<std::string>();
				if (!name.empty() && U.named.find(name) != U.named.end()) {
					return Error(fmt::format("object named {} already exists", name));
				}
				auto type = j.at("type").get<std::string>();
				auto iter = classInfo.find(type);
				if(iter == classInfo.end())
					return Error(fmt::format("unknown type", type));
				auto r = create(iter->second, name);
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
						fmt::print("reset: {}\n", t->name);
						auto o = U.named[t->name];
						i.reset(o);
						state.visited.insert(o);
					}
					else {
						resetReferences(i.object, state);
					}
				}
			}

			Result<Object*> deserialize(const json& j) {
				DeserializationState state;
				auto r = _deserialize(j,state);
				if (!r)return r;
				resetReferences(r.value(), state);
				return r.value();
			}

			template<typename T>
			Result<T*> deserialize(const json& j) {
				auto type = j.at("type").get<std::string>();
				if (type != T::__classinfo__()->name()) {
					return Error(
						fmt::format("Type mismatch: expect {} but have {}\n",
							T::__classinfo__()->name(),
							type));
				}
				Class* info = T::__classinfo__();
				auto name = j.at("name").get<std::string>();
				auto r = deserialize(j);
				if (r.hasValue()) {
					return (T*)r.value();
				}
				return Error(r.error());
			}
			
		};
	}
}

#endif