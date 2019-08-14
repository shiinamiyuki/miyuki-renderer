#ifndef MIYUKI_MATERIAL_H
#define MIYUKI_MATERIAL_H
#include <miyuki.h>
#include <math/transform.h>
#include <core/intersection.hpp>
#include <core/shaders/shader.h>
#include <core/memory.h>

namespace Miyuki {
	namespace Core {
		class BSDFImpl;
		struct BSDFCreationContext {
			MemoryArena* arena;
			ShadingPoint shadingPoint;
			template<class T, class... Args >
			T* alloc(Args&& ... args) {
				T* p = (T*)arena->alloc(sizeof(T));
				new (p) T(args...);
				return p;
			}
			BSDFCreationContext(const ShadingPoint& p, MemoryArena* arena) :shadingPoint(p), arena(arena) {}
		};

		class Material : public Reflective {
		public:
			MYK_ABSTRACT(Material);
			Box<Shader> emission;
			Box<Shader> normalMap;
			virtual BSDFImpl* createBSDF(BSDFCreationContext&)const = 0;
		};
		MYK_REFL(Material, (Reflective), (emission)(normalMap));
	}
}

#endif