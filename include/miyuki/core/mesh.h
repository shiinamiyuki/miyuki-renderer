#ifndef MIYUKI_MESH_H
#define MIYUKI_MESH_H

#include <miyuki.h>
#include <core/ray.h>
#include <core/intersection.hpp>
#include <embree3/rtcore.h>

#define USE_EMBREE_GEOMETRY 1
namespace Miyuki {
	class Light;

	struct Mesh;

	class Material;

	class Accelerator;

	struct Primitive {
		uint32_t vertices[3];
		uint32_t normals[3];
		Point2f textureCoord[3];

		Mesh* instance;
		uint16_t nameId;
		Float area()const {
			return Vec3f::cross((v(1) - v(0)), v(2) - v(0)).length() / 2;
		}

		Primitive();

		Vec3f v(int32_t) const;

		const Vec3f& n(int32_t) const;

		Vec3f Ns(const Point2f& uv) const;

		Material* material() const;

		const std::string& name() const;

		bool intersect(const Ray&, Intersection*) const;

		Light* light() const;

		void setLight(Light* light);

		Vec3f Ng() const {
			auto edge1 = v(1) - v(0);
			auto edge2 = v(2) - v(0);
			return Vec3f::cross(edge1, edge2).normalized();
		}
	};

	class EmbreeScene;

	struct Mesh {
		std::string name;
		Transform transform;
		std::vector<Vec3f> vertices, normals;
		std::vector<Primitive> primitives;
		std::vector<std::string> names;
		std::vector<std::shared_ptr<Material>> materials;
		int geomId = -1;
		EmbreeScene* accelerator;
		uint32_t vertexCount = 0;
#if USE_EMBREE_GEOMETRY == 1
		RTCGeometry rtcGeometry = nullptr;
#endif

		Mesh(const std::string& filename);

		std::shared_ptr<Mesh> instantiate(const std::string& name, const Transform& transform = Transform()) const;

		void resetTransform(const Transform& T);

		std::unordered_map<const Primitive*, Light*> lightMap;

		~Mesh();
	};
}
#endif