#pragma once
#include <reflection.h>
#include <core/mesh.h>

namespace Miyuki {
	namespace Core {
		class Accelerator : public Reflective{
		public:
			MYK_INTERFACE(Accelerator);
			virtual void addMesh(std::shared_ptr<Mesh> mesh, uint32_t id) = 0;

			virtual bool intersect(const Ray& ray, Intersection* isct) = 0;

			virtual void intersect4(const Ray4& ray, Intersection4* isct) { }

			virtual void intersect8(const Ray8& ray, Intersection8* isct) { }

			virtual void detachGeometry(int geomId) = 0;

			virtual Bound3f getWorldBound()const = 0;
		};
		MYK_REFL(Accelerator, (Reflective), MYK_REFL_NIL);
	}
}