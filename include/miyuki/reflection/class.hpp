#ifndef MIYUKI_REFLECTION_CLASS_HPP
#define MIYUKI_REFLECTION_CLASS_HPP
#include <miyuki.h>
#include <utils/noncopymovable.hpp>
namespace Miyuki {
	namespace Reflection {
		class Object;
		struct Class : NonCopyMovable{
			using Constructor = std::function<Object* ()>;
			struct {
				const Class* base;
				Constructor ctor; 
			} classInfo;
			const char* _name;
			const char* name() {
				return _name;
			}
			Object* create() { return classInfo.ctor(); }
		};
	}
}

#endif