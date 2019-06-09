#ifndef MIYUKI_REFLECTION_GC_HPP
#define MIYUKI_REFLECTION_GC_HPP

#include "reflection.hpp"
namespace Miyuki {
	namespace Reflection {
		/*
		Basic Garbage Collector
		*/
		class GC {
		protected:
			struct USet {
				std::set<Object*> anonymous;
				std::unordered_map<std::string, Object*> named;
				void foreach(const std::function<void(Object*)>& f) {
					for (auto& i : anonymous)f(i);
					for (auto& i : named)f(i.second);
				}
			}U;// the universal set
			std::set<Object*> root; //the root set
			std::unordered_map<std::string, Class*> classInfo;

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
			template<class T>
			GC& registerClass() {
				Class* info = T::__classinfo__();
				classInfo[info->name()] = info;
				return *this;
			}
			template<class T>
			T* create(const std::string& name) {
				Class* info = T::__classinfo__();
				if (!name.empty() && U.named.find(name) != U.named.end()) {
					throw std::runtime_error(fmt::format("object named `{}` already exists", name));
				}
				T* object = (T*)classInfo.at(info->name())->create(name);
				addObject(object);
				return object;
			}
			template<class T, typename... Args>
			T* create(const std::string& name, Args... args) {
				Class* info = T::__classinfo__();
				if (!name.empty() && U.named.find(name) != U.named.end()) {
					return nullptr;
				}
				T* object = (T*)classInfo.at(info->name())->create(name);
				addObject(object);
				object.init(args...);
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
				root.erase(object);
			}
		};

	}
}


#endif