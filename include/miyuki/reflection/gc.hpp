#ifndef MIYUKI_REFLECTION_GC_HPP
#define MIYUKI_REFLECTION_GC_HPP

#include "reflection.hpp"
namespace Miyuki {
	namespace Reflection {
		/*
		Basic Garbage Collector
		*/
		class GC {
			struct USet {
				std::set<Object*> anonymous;
				std::unordered_map<std::string, Object*> named;
				void foreach(const std::function<void(Object*)>& f) {
					for (auto& i : anonymous)f(i);
					for (auto& i : named)f(i->second);
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
						stack.push(i);
					}
				}
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
						delete obj;
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
						delete obj;
						iter = U.named.erase(iter);
					}
				}
			}

		public:
			template<class T>
			void registerClass() {
				Class* info = T::__classinfo__();
				classInfo[info->name()] = info; 
			}
			
			template<class T, typename... Args> 
			T* create(const std::string & name, Args args) {
				Class* info = T::__classinfo__();
				if (!name.empty() && U.named.find(name) != U.named.end()) {
					return nullptr;
				}
				T* object = (T*)classInfo[info->name()]->create(name);
				if (name.empty()) {
					U.anonymous.insert(object);
				}
				else {
					U.named[name] = object; 
				}
				object.init(args...);
				return object;
			}

			/*
			WARNING: Not thread safe, must be manually called
			This GC performs mark&sweep
			*/
			void GC() {
				mark();
				sweep();
			}
		};

	}
}


#endif