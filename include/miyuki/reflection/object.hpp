#ifndef MIYUKI_REFLECTION_OBJECT_HPP
#define MIYUKI_REFLECTION_OBJECT_HPP
#include <string>
#include <vector>
#include <utils/noncopymovable.hpp>
#include "class.hpp"
namespace Miyuki {

	namespace Reflection {
#define MYK_NULL_CLASS Null
#define MYK_CLASS_TYPE_INFO(classname, basename) \
		static Class* __class__()const{\
			static Class* info = nullptr;\
			std::call_once([&](){info = new Class();});\
			info._name = #classname; \
			info.classInfo.base = basename.__class__();\
			info.ctor = [=](){return new classname(info);}\
			return info; \
		}
		class Object;
		struct Property {
			Object* object = nullptr;
			std::string name;
			Property(Object* object, const std::string& name)
				: object(object), name(name) {}
			Property(const std::string& name): name(name) {}
			virtual const Property& get()const = 0;
			virtual Property& get() = 0;
		};
		class Null {
		public:
			static Class * __class__()const {
				static Class* info = nullptr; 
				std::call_once([&]() {info = new Class(); }); 
				info._name ="Null"; 
				info.classInfo.base = info; 
				info.ctor = [=]() {return nullptr; }
				return info; 
			}
		};
		template<class T>
		struct PropertyT {
			static_assert(std::is_base_of<PropertyT, T>::value);
			PropertyT(T* object, const std::string& name)
				:Property(object, name) {}
			PropertyT(const std::string& name) :Property(name) {}
			T* operator->() {
				return object;
			}
			T& operator *() {
				return *object;
			}
			const T& operator * () const {
				return *object;
			}
			virtual const Property& get()const { return *this; }
			virtual Property& get() { return *this; }
		};

		// NonCopyMovable gaurantees that during the object's lifetime,
		// a reference to the object will never fail
		class Object : NonCopyMovable {
		protected:
			Class* _class;
			std::string _name;
			Object(const Class *_class):_class(_class){}
		public:
			
			const char* typeName()const {
				return _class->name();
			}
			bool hasBase()const {
				return _class->classInfo.base != nullptr;
			}
			const char* baseName()const {
				return _class->classInfo.base->name();
			}
			Class& getClass() {
				return *_class;
			}
			bool sameType(const Object& rhs)const {
				return _class == rhs._class;
			}
			virtual bool isPrimitive()const { return false; }
			const std::string& name()const { return _name; }
			virtual const std::vector<Property*> getProperties()const = 0;
		};
	}
}

#endif