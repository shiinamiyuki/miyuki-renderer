#pragma once

namespace Miyuki {
	namespace Core {
		class Allocator {
		public:
			virtual void* alloc(size_t) = 0;
			template<class T>
			T* allocT() {
				return (T*)alloc(sizeof(T));
			}
			virtual void free() = 0;
			virtual ~Allocator() = default;
		};
	}
}