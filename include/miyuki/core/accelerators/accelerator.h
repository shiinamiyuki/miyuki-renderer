#pragma once
#include <miyuki.h>
#include <core/mesh.h>

namespace Miyuki {
	namespace Core {
		class Accelerator {
		public:
			virtual void addMesh(std::shared_ptr<Mesh> mesh, int id) = 0;

			virtual bool intersect(const Ray& ray, Intersection* isct) = 0;

			//virtual void detachGeometry(int geomId) = 0;
		};
	}
}