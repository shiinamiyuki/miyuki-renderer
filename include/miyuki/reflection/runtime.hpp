#ifndef MIYUKI_REFLECTION_RUNTIME_HPP
#define MIYUKI_REFLECTION_RUNTIME_HPP
#include "reflection.hpp"
#include "gc.hpp"
namespace Miyuki {
	namespace Reflection {
		/*
		Enables serialization/deserialization
		Manages all objects
		*/
		class Runtime : public GC {
			struct DeserializationState {
				std::unordered_map<const Property*, std::string> map;
			};
		public:
			Result<Object*> _deserialize(const json& j) {
				// null or references are skipped
				if (j.is_null() || j.is_string())
					return nullptr;
				auto name = j.at("name").get<std::string>();
				if (!name.empty() && U.named.find(name) != U.named.end()) {
					return Error(fmt::format("object named {} already exists", name));
				}
				auto type = j.at("type").get<std::string>();
				auto iter = classInfo.find(type);
				if(iter == classInfo.end())
					return Error(fmt::format("unknown type", type));
				auto object = iter->second->create(name);
				object->deserialize(j, [this](const json& js)->Result<Object*> {
					return _deserialize(js);
				});
				return object;
			}
			Result<Object*> deserialize(const json& j) {
				return _deserialize(j);
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