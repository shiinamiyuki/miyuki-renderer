#ifndef MIYUKI_REFLECTION_CLASS_HPP
#define MIYUKI_REFLECTION_CLASS_HPP
#include <miyuki.h>
#include <utils/noncopymovable.hpp>
namespace Miyuki {
	namespace Reflection {
		class Object;
		struct Class : NonCopyMovable{
			using Constructor = std::function<Object* (const std::string&)>;
			struct {
				const Class* base;
				Constructor ctor; 
			} classInfo;
			const char* _name;
			const char* name() const{
				return _name;
			}
			Object* create(const std::string & n="")const { return classInfo.ctor(n); }
		};
	}
}

#endif