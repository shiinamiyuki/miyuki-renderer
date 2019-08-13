#ifndef MIYUKI_MATERIAL_H
#define MIYUKI_MATERIAL_H
#include <miyuki.h>
#include <math/transform.h>
#include <core/intersection.hpp>
#include <core/shaders/shader.h>
#include <core/memory.h>
#include <core/graphcompiler.h>

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

		class Material : public Reflective, public Compilable {
		public:
			MYK_ABSTRACT(Material);
			Box<Shader> emission;
			virtual BSDFImpl* createBSDF(BSDFCreationContext&)const = 0;
			int compileToKernelMaterial(GraphCompiler& compiler)const {
				compile(compiler);
				return compiler.getLastAddedMaterialId();
			}
		};
		MYK_REFL(Material, (Reflective), (emission));
	}
}

#endif