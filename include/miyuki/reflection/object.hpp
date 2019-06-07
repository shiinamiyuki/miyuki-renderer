#ifndef MIYUKI_REFLECTION_OBJECT_HPP
#define MIYUKI_REFLECTION_OBJECT_HPP
#include <miyuki.h>
#include <utils/noncopymovable.hpp>
#include "class.hpp"
namespace Miyuki {

	namespace Reflection {
		struct SerializationState {
			std::set<std::string> visited;
		};
		template<int>
		struct UID {};
#define MYK_NULL_CLASS Null
#define MYK_CLASS_TYPE_INFO(classname, basename) \
		enum {_propertyIdx = __COUNTER__}; \
		using BaseT = basename; \
		static Miyuki::Reflection::Class* __classinfo__(){\
			static Miyuki::Reflection::Class* info = nullptr;\
			static std::once_flag flag;\
			std::call_once(flag,[&](){info = new Miyuki::Reflection::Class();\
			info->_name = #classname; \
			info->classInfo.base = basename::__classinfo__();\
			info->classInfo.ctor = [=](const std::string& n){return new classname(info, n);};});\
			return info; \
		}
		class Object;
		struct Property {
			Object* object = nullptr;
			std::string name;
			Property(Object* object, const std::string& name)
				: object(object), name(name) {}
			Property(const std::string& name) : name(name) {}
			virtual const Property& get()const = 0;
			virtual Property& get() = 0;
			Property& operator = (Object* o) { object = o; return *this; }
		};
		class Null {
		public:
			static Class* __classinfo__() {
				static Class* info = nullptr;
				static std::once_flag flag;
				std::call_once(flag, [&]() {info = new Class();
				info->_name = "Null";
				info->classInfo.base = nullptr;
				info->classInfo.ctor = [=](const std::string&) {return nullptr; };
					});
				return info;
			}
		};
		template<class T>
		struct PropertyT :public Property{
			static_assert(std::is_base_of<Object, T>::value);
			PropertyT(T* object, const std::string& name)
				:Property(object, name) {}
			PropertyT(const std::string& name) :Property(name) {}
			T* operator->() {
				return (T*)object;
			}
			const T* operator->()const {
				return (T*)object;
			}
			T& operator *() {
				return *object;
			}
			const T& operator * () const {
				return *object;
			}
			virtual const Property& get()const { return *this; }
			virtual Property& get() { return *this; }
			PropertyT& operator = (T* o) { object = o; return *this; }
		};

		class GC;
		// NonCopyMovable gaurantees that during the object's lifetime,
		// a reference to the object will never fail
		class Object : NonCopyMovable {
			friend class GC;
			bool _marked = false;		
			void mark() { _marked = true; }
			bool marked()const { return _marked; }
			void unmark() { _marked = false; }
		protected:
			Class* _class;
			std::string _name;
			Object(Class* _class, const std::string& name = "") :_class(_class), _name(name) {}
		public:
			
			static Class* __classinfo__() {
				static Class* info = nullptr;
				static std::once_flag flag;
				std::call_once(flag, [&]() {
					info = new Class();
					info->_name = "Miyuki::Reflection::Object";
					info->classInfo.base = Null::__classinfo__();
					info->classInfo.ctor = [=](const std::string& n) {return new Object(info, n); };
					});
				return info;
			}
			const char* typeName()const {
				return _class->name();
			}
			bool hasBase()const {
				return getClass().classInfo.base != nullptr;
			}
			const char* baseName()const {
				return getClass().classInfo.base->name();
			}
			const Class& getClass()const {
				return *_class;
			}
			bool sameType(const Object& rhs)const {
				return _class == rhs._class;
			}
			virtual bool isPrimitive()const { return false; }
			const std::string& name()const { return _name; }
			virtual const std::vector<const Property*> getProperties()const {
				return {};
			}
			void serialize(json& j)const {
				SerializationState state;
				serialize(j, state);
			}
			virtual void serialize(json& j, SerializationState&)const {
				j["name"] = name();
				j["type"] = typeName();
				if (!isPrimitive()) {
					j["properties"] = json::object();
					for (auto _i : getProperties()) {
						auto& i = _i->get();
						if (!i.object)
							j["properties"][i.name] = {};
						else {
							i.object->serialize(j["properties"][i.name]);
						}
					}
				}
			}
			virtual std::vector<Object*> getReferences()const {
				auto v = getProperties();
				decltype(getReferences()) result;
				for (auto i : v) {
					if(i->object)
						result.push_back(i->object);
				}
				return result;
			}
		};
		template<class T, int Idx>
		struct _GetPropertiesHelperIdx {
			static void _GetProperties(const T& obj, std::vector<const Property*>& vec) {
				vec.push_back(&obj.getProperty(UID<T::_propertyCount - Idx - 1>()));
				_GetPropertiesHelperIdx<T, Idx - 1>::_GetProperties(obj, vec);
			}
		};
		template<class T>
		struct _GetPropertiesHelperIdx<T, 0> {
			static void _GetProperties(const T & obj,std::vector<const Property*>& vec) {
				vec.push_back(&obj.getProperty(UID<T::_propertyCount - 1>()));
			}
		};
		template<class T, int Count>
		struct _GetPropertiesHelper {
			static void _GetProperties(const T& obj, std::vector<const Property*>& vec) {
				_GetPropertiesHelperIdx<T, Count - 1>::_GetProperties(obj, vec);
			}
		};
		template<class T>
		struct _GetPropertiesHelper<T, 0> {
			static void _GetProperties(const T& obj, std::vector<const Property*>& vec) {
			}
		};
	} 
}



#endif