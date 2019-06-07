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
		class Runtime: public GC {
		public: 
			Object* _deserialize(const json& j) {
				if (j.is_null())
					return nullptr;
				auto name = j.at("name").get<std::string>();
				auto type = j.at("type").get<std::string>();
				auto object = classInfo.at(type)->create(name);
				object->deserialize(j, [this](const json& js)->Object* {
					return _deserialize(js);
					});
				return object;
			}
			Object* deserialize(const json& j) {
				auto object = _deserialize(j);
				addObject(object);
				return object;
			}

			template<typename T>
			T* deserialize(const json& j) {
				auto type = j.at("type").get<std::string>();
				if (type != T::__classinfo__()->name()) {
					throw std::runtime_error(
						fmt::format("Type mismatch: expect {} but have {}\n",
							T::__classinfo__()->name(),
							type));
				}
				Class* info = T::__classinfo__();
				auto name = j.at("name").get<std::string>();
				return (T*)deserialize(j);
			}
		};
	}
}

#endif