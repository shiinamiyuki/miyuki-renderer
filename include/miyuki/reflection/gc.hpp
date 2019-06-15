#ifndef MIYUKI_REFLECTION_GC_HPP
#define MIYUKI_REFLECTION_GC_HPP

#include "reflection.hpp"
#include <boost/uuid/uuid.hpp>          
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp> 

namespace Miyuki {
	namespace Reflection {
		template<class T>
		class LocalObject {
			GC* runtime = nullptr;
			T* object = nullptr;
		public:
			LocalObject() {}
			LocalObject(GC& runtime) :runtime(&runtime), object(nullptr) {}
			LocalObject(GC& runtime, T* object) :runtime(&runtime), object(object) {
				if (object)
					this->runtime->addRoot(object);
			}
			LocalObject(const LocalObject& rhs) :runtime(rhs.runtime), object(rhs.object) {
				if (object)
					runtime->addRoot(object);
			}
			LocalObject& operator=(const LocalObject& rhs) {
				if(object)
					runtime->removeRoot(object);
				object = rhs.object;
				runtime = rhs.runtime;
				if (object)
					runtime->addRoot(object);
				return *this;
			}
			LocalObject& operator=(T * rhs) {
				if (object)
					runtime->removeRoot(object);				
				object = rhs;
				if (object)
					runtime->addRoot(object);
				return *this;
			}
			~LocalObject() {
				if (object)
					runtime->removeRoot(object);
			}
			T* operator->() {
				return object;
			}
			T& operator *() {
				return *object;
			}
			operator T* () {
				return object;
			}
		};

		/*
		Basic Garbage Collector
		*/
		class GC {
			boost::uuids::random_generator UUIDGenerator;
			std::atomic<uint64_t> memoryAllocated;
		protected:
			struct USet {
				std::set<Object*> anonymous;
				std::unordered_map<std::string, Object*> named;
				void foreach(const std::function<void(Object*)>& f) {
					for (auto& i : anonymous)f(i);
					for (auto& i : named)f(i.second);
				}
			}U;// the universal set
			std::multiset<Object*> root; //the root set
			static std::unordered_map<std::string, Class*> classInfo;

			void mark(Object* record) {
				std::stack<Object*> stack;
				stack.push(record);
				while (!stack.empty()) {
					auto o = stack.top();
					stack.pop();
					o->mark();
					for (auto& i : o->getReferences()) {
						stack.push(i.object);
					}
				}
			}

			void destroy(Object* object) {
				memoryAllocated += object->getClass().classInfo.size;
				delete object;
			}
			void mark() {
				for (auto& i : root) {
					mark(i);
				}
			}

			void sweep() {
				for (auto iter = U.anonymous.begin(); iter != U.anonymous.end();) {
					auto obj = *iter;
					if (obj->marked()) {
						iter++;
						obj->unmark();
					}
					else {
						destroy(obj);
						iter = U.anonymous.erase(iter);
					}
				}
				for (auto iter = U.named.begin(); iter != U.named.end();) {
					auto obj = iter->second;
					if (obj->marked()) {
						iter++;
						obj->unmark();
					}
					else {
						destroy(obj);
						iter = U.named.erase(iter);
					}
				}
			}
			void addObject(Object* object) {
				if (!object)return;
				const auto& name = object->name();
				if (name.empty()) {
					U.anonymous.insert(object);
				}
				else {
					U.named[name] = object;
				}
			}
		public:
			size_t estimiatedMemoryUsage() {
				return memoryAllocated;
			}
			GC() :memoryAllocated(0) {}
			template<class T>
			GC& registerClass() {
				static std::once_flag flag;
				std::call_once(flag, []() {
					Class* info = T::__classinfo__();
					classInfo[info->name()] = info;
				});
				return *this;
			}

			Result<Object*> create(Class* info, const std::string& name) {
				if (!name.empty() && U.named.find(name) != U.named.end()) {
					return Error(fmt::format("object named `{}` already exists", name));
				}
				auto obj = info->create(name);
				memoryAllocated += info->classInfo.size;
				addObject(obj);
				return obj;
			}

			template<class T>
			Result<T*> create(const std::string& name) {
				registerClass<T>();
				Class* info = T::__classinfo__();
				auto r = create(info, name);
				if (!r)return r.error();
				auto object = (T*)r.value();
				return object;
			}

			template<class T, typename... Args>
			Result<T*> create(const std::string& name, Args... args) {
				registerClass<T>();
				Class* info = T::__classinfo__();
				auto r = create(info, name);
				if (!r)return r.error();
				auto object = (T*)r.value();
				object->init(args...);
				return object;
			}

			/*
			WARNING: Not thread safe, must be manually called
			This GC performs mark&sweep
			*/
			void collect() {
				mark();
				sweep();
			}

			~GC() {
				U.foreach([](Object* obj) {
					delete obj;
				});
			}

			template<class T>
			void addRoot(T* object) {
				root.insert(object);
			}

			template<class T>
			void removeRoot(T* object) {
				auto it = root.find(object);
				assert(it != root.end());
				root.erase(it);
			}
			std::string generateUUID() {
				std::stringstream s;
				s << UUIDGenerator();
				return s.str();
			}
			template<typename T>
			LocalObject<T> New() {
				auto object = create<T>(generateUUID());
				if (!object) {
					throw std::runtime_error(object.error().what());
				}
				return LocalObject<T>(*this, object.value());
			}
			template<typename T, typename... Args>
			LocalObject<T> New(Args... args) {
				auto object = create<T>(generateUUID(), args...);
				if (!object) {
					throw std::runtime_error(object.error().what());
				}
				return LocalObject<T>(*this, object.value());
			}
			template<typename T>
			LocalObject<T> NewNamed(const std::string& name) {
				auto object = create<T>(name);
				if (!object) {
					throw std::runtime_error(object.error().what());
				}
				return LocalObject<T>(*this, object.value());
			}
			template<typename T, typename... Args>
			LocalObject<T> NewNamed(const std::string& name, Args... args) {
				auto object = create<T>(name, args...);
				if (!object) {
					throw std::runtime_error(object.error().what());
				}
				return LocalObject<T>(*this, object.value());
			}
		};

	}
}


#endif