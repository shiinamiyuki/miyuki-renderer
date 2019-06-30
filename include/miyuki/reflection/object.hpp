#ifndef MIYUKI_REFLECTION_OBJECT_HPP
#define MIYUKI_REFLECTION_OBJECT_HPP
#include <miyuki.h>
#include <utils/noncopymovable.hpp>
#include <utils/result.hpp>
#include "class.hpp"
namespace Miyuki {

	namespace Reflection {
		struct SerializationState {
			std::set<const Object*> visited;
			Optional<Error> error;
			void add(const Object* o) { visited.insert(o); }
			bool has(const Object* o) { return visited.find(o) != visited.end(); }
			bool hasError()const {
				return error.has_value();
			}
		};

		template<int>
		struct UID {};
#define MYK_NULL_CLASS Null
#define MYK_CLASS_TYPE_INFO(classname, basename) \
		enum {_propertyIdx = __COUNTER__}; \
		using BaseT = basename;using ThisT = classname; \
		static Miyuki::Reflection::Class* __classinfo__(){\
			static Miyuki::Reflection::Class* info = nullptr;\
			static std::once_flag flag;\
			std::call_once(flag,[&](){info = new Miyuki::Reflection::Class();\
			info->_name = #classname; \
			info->classInfo.size = sizeof(ThisT); \
			info->classInfo.base = basename::__classinfo__();\
			info->classInfo.ctor = [=](const Miyuki::Reflection::UUID& n)->Miyuki::Reflection::Object*\
			{return new classname(info, n);};});\
			return info; \
		}
#define MYK_FINAL_CLASS_TYPE_INFO(classname, basename) \
		enum {_propertyIdx = __COUNTER__}; \
		using BaseT = basename;using ThisT = classname; \
		static Miyuki::Reflection::Class* __classinfo__(){\
			static Miyuki::Reflection::Class* info = nullptr;\
			static std::once_flag flag;\
			std::call_once(flag,[&](){info = new Miyuki::Reflection::Class();\
			info->_name = #classname; info->classInfo.isFinal = true;\
			info->classInfo.size = sizeof(ThisT); \
			info->classInfo.base = basename::__classinfo__();\
			info->classInfo.ctor = [=](const Miyuki::Reflection::UUID& n)->Miyuki::Reflection::Object*\
			{return new classname(n);};});\
			return info; \
		}
		class Object;
		struct Property {
			mutable Object* object = nullptr;
			std::string name;
			Property(Object* object, const std::string& name)
				: object(object), name(name) {}
			Property(const std::string& name) : name(name) {}
			virtual const Property& get()const = 0;
			virtual Property& get() = 0;
			Property& operator = (Object* o) { object = o; return *this; }
			bool isNull()const {
				return object == nullptr;
			}
			operator bool()const {
				return !isNull();
			}
			bool operator == (const Object* o)const { return o == object; }
		};
		class Null {
		public:
			static Class* __classinfo__() {
				static Class* info = nullptr;
				static std::once_flag flag;
				std::call_once(flag, [&]() {info = new Class();
				info->_name = "Null";
				info->classInfo.size = sizeof(Null);
				info->classInfo.base = nullptr;
				info->classInfo.ctor = [=](const UUID&) {return nullptr; };
				});
				return info;
			}
		};
		template<class T>
		struct PropertyT :public Property {
			//static_assert(std::is_base_of<Object, T>::value, "Invalid template argument T");
			PropertyT(T* object, const std::string& name)
				:Property(object, name) {}
			PropertyT(const std::string& name) :Property(name) {}
			T* operator->() {
				return static_cast<T*>(object);
			}
			const T* operator->()const {
				return static_cast<T*>(object);
			}
			T& operator *() {
				return *static_cast<T*>(object);
			}
			const T& operator * () const {
				return *static_cast<T*>(object);
			}
			virtual const Property& get()const { return *this; }
			virtual Property& get() { return *this; }
			PropertyT& operator = (T* o) { object = o; return *this; }
			/*Class* getDeclaredClass() {
				return T::__classinfo__();
			}*/
		};

		class GC;
		class Visitor;



		// NonCopyMovable gaurantees that during an object's lifetime,
		// a reference to the object will never fail
		class Object : NonCopyMovable {
			friend class GC;
			bool _marked = false;
			void mark() { _marked = true; }
			bool marked()const { return _marked; }
			void unmark() { _marked = false; }
		protected:
			GC* allocator = nullptr;
			Class* _class;
			UUID _id;
			Object(Class* _class, const UUID& id) :_class(_class), _id(id) {}
		public:
			static Class* __classinfo__() {
				static Class* info = nullptr;
				static std::once_flag flag;
				std::call_once(flag, [&]() {
					info = new Class();
					info->classInfo.size = sizeof(Object);
					info->_name = "Miyuki::Reflection::Object";
					info->classInfo.base = Null::__classinfo__();
					info->classInfo.ctor = [=](const UUID& id) {return new Object(info, id); };
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
			bool isDerivedOf(const Class* c)const {
				auto p = &getClass();
				while (p && p != c) {
					p = p->classInfo.base;
				}
				return p != nullptr;
			}
			bool isBaseOf(const Class* c) const {
				while (c && c != &getClass()) {
					c = c->classInfo.base;
				}
				return c != nullptr;
			}

			bool isBaseOf(Object* object)const {
				const Class* p = &object->getClass();
				return isBaseOf(p);
			}

			bool isSameType(const Object& rhs)const {
				return _class == rhs._class;
			}
			bool isSameType(const Object* rhs)const {
				return rhs && _class == rhs->_class;
			}
			virtual bool isPrimitive()const { return false; }

			const UUID& id()const { return _id; }

			virtual const std::vector<const Property*> getProperties()const {
				return {};
			}

			Optional<Error> serialize(json& j)const {
				SerializationState state;
				serialize(j, state);
				return state.error;
			}

			virtual void serialize(json& j, SerializationState& state)const {
				if (state.hasError())return;
				// never serialize the same object twice
				if (state.has(this)) {
					j = id().str();
					return;
				}
				state.add(this);
				j["id"] = id().str();
				j["type"] = typeName();
				if (!isPrimitive()) {
					j["properties"] = json::object();
					for (auto _i : getProperties()) {
						auto& i = _i->get();
						if (!i.object)
							j["properties"][i.name] = {};
						else {
							i.object->serialize(j["properties"][i.name], state);
						}
					}
				}
			}
			struct Reference {
				// WARNING: reset can only be used if the caller is sure that
				// the object hasn't changed since getReference is called 
				// can be fixed in the future
				std::function<void(Object*)> reset;
				Object* object;

				Reference(const decltype(reset)& f, Object* object) :object(object), reset(f) {}
			};
			virtual std::vector<Reference> getReferences() {
				auto v = getProperties();
				decltype(getReferences()) result;
				for (auto i : v) {
					if (i->object)
						result.push_back(Reference([=](Object* o) {i->object = o; }, i->object));
				}
				return result;
			}
			using Resolver = std::function<Result<Object*>(const json&)>;
			virtual void deserialize(const json& j, const Resolver& resolve) {
				for (auto i : getProperties()) {
					if (auto r = resolve(j.at("properties").at(i->name))) {
						i->object = r.value();
					}
					else {
						throw std::runtime_error(r.error().what());
					}
				}
			}
			Result<const Property*> getPropertyByName(const std::string& name)const {
				for (auto i : getProperties()) {
					if (i->name == name) {
						return i;
					}
				}
				return Error(
					fmt::format("{} has no property named {}\n", typeName(), name));
			}

			template<class T>
			Result<T*> cast() {
				// up-cast
				if (&getClass() == T::__classinfo__() || isDerivedOf(T::__classinfo__())) {
					return (T*)this;
				}
				else {
					return Error(
						fmt::format("Cannot cast from {} to {}", typeName(), T::__classinfo__()->name())
					);
				}
			}
			template<class T>
			T* staticCast() {
				return static_cast<T*>(this);
			}
			virtual size_t hashCode()const {
				return (size_t)__classinfo__();
			}
			virtual bool equals(Object* obj)const {
				return obj == this;
			}
			static bool equals(Object* a, Object* b) {
				if (a == b)return true;
				if (!a || !b)return false;
				return a->equals(b); 
			}
			inline void accept(Visitor&);
			void init() {}
		};
		template<class T>
		Result<T*> Cast(Object* p) {
			if (!p)return nullptr;
			return p->cast<T>();
		}
		template<class T>
		T* StaticCast(Object* p) {
			if (!p)return nullptr;
			return p->staticCast<T>();
		}
#define __MYK_GET_PROPERTY_HELPER \
		template<class T, int Idx>\
		struct _GetPropertiesHelperIdx {\
			static void _GetProperties(const T& obj, std::vector<const Miyuki::Reflection::Property*>& vec) {\
				vec.push_back(&obj.getProperty(Miyuki::Reflection::UID<T::_propertyCount - Idx - 1>()));\
				_GetPropertiesHelperIdx<T, Idx - 1>::_GetProperties(obj, vec);\
			}\
		};\
		template<class T>\
		struct _GetPropertiesHelperIdx<T, 0> {\
			static void _GetProperties(const T & obj,std::vector<const Miyuki::Reflection::Property*>& vec) {\
				vec.push_back(&obj.getProperty(Miyuki::Reflection::UID<T::_propertyCount - 1>()));\
			}\
		};\
		template<class T, int Count>\
		struct _GetPropertiesHelper {\
			static void _GetProperties(const T& obj, std::vector<const Miyuki::Reflection::Property*>& vec) {\
				_GetPropertiesHelperIdx<T, Count - 1>::_GetProperties(obj, vec);\
			}\
		};\
		template<class T>\
		struct _GetPropertiesHelper<T, 0> {\
			static void _GetProperties(const T& obj, std::vector<const Miyuki::Reflection::Property*>& vec) {\
			}\
		};
		class Visitor {
			using VisitFunc = std::function<void(Object*)>;
			std::unordered_map<const Reflection::Class*, VisitFunc> funcs;
		protected:
			template<class T>
			void visit(std::function<void(T*)> func) {
				funcs[T::__classinfo__()] = [=](Object* object) {
					func(Reflection::StaticCast<T>(object));
				};
			}
		public:
			inline void visit(Object* object);
		};
		inline void Object::accept(Visitor& visitor) {
			visitor.visit(this);
		}

		// first check if there is an `overload` for object's type
		// if not check its parent 
		// until we have reached the top level.
		inline void Visitor::visit(Object* object) {
			if (!object)return;
			const Class* _class = &object->getClass();
			auto classname = _class->name();
			while (true) {
				auto iter = funcs.find(_class);
				if (iter != funcs.end()) {
					iter->second(object);
					return;
				}
				if (_class->classInfo.base) {
					_class = _class->classInfo.base;
				}
				else {
					throw std::runtime_error(
						fmt::format("No matching visitor can be called upon {}", classname));
				}
			}
		}
	}
}



#endif