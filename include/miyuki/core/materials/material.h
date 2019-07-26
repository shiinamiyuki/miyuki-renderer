#ifndef MIYUKI_MATERIAL_H
#define MIYUKI_MATERIAL_H
#include <miyuki.h>
#include <math/transform.h>
#include <core/intersection.hpp>
#include <core/shaders/shader.h>
#include <core/memory.h>

namespace Miyuki {
	namespace Core {

		struct BSDFCreationContext {
			MemoryArena* arena;
			ShadingPoint shadingPoint;
			template<class T, class... Args >
			T* alloc(Args&&... args) {
				T * p =  (T*)arena->alloc(sizeof(T));
				new (p) T(args...);
				return p;
			}
		};

		class Material : public Reflective {
		public:
			MYK_INTERFACE(Material);
			virtual Shader* emission()const = 0;
			virtual BSDF* createBSDF(BSDFCreationContext&)const = 0;
		};
		MYK_REFL(Material, (Reflective), MYK_REFL_NIL);		
	}
}

#endif