#ifndef MIYUKI_MESH_H
#define MIYUKI_MESH_H

#include <miyuki.h>
#include <core/ray.h>
#include <core/intersection.hpp>
#include <embree3/rtcore.h>

#define USE_EMBREE_GEOMETRY 1
namespace Miyuki {
	namespace Core {
		class Light;

		class Mesh;

		class Material;

		class Accelerator;

		struct Primitive {
			uint32_t primitiveId;
			Point2f textureCoord[3];

			Mesh* instance;
			uint16_t nameId;
			Float area()const {
				return Vec3f::cross((v(1) - v(0)), v(2) - v(0)).length() / 2;
			}

			Primitive() : instance(nullptr), nameId(-1) {

			}

			inline Vec3f v(int32_t) const;

			inline const Vec3f& n(int32_t) const;

			inline Vec3f Ns(const Point2f& uv) const;

			inline Material* material() const;

			inline const std::string& name() const;

			inline bool intersect(const Ray&, Intersection*) const;

			inline Light* light() const;

			inline void setLight(Light* light);

			Vec3f Ng() const {
				auto edge1 = v(1) - v(0);
				auto edge2 = v(2) - v(0);
				return Vec3f::cross(edge1, edge2).normalized();
			}
		};

		class EmbreeScene;

		class Mesh {
		public:
			friend struct Primitive;
			std::string name;
			std::string filename;
			Transform transform;

			std::vector<std::string> names;
			std::vector<Material*> materials;
			std::weak_ptr<Mesh> parent;
			int geomId = -1;
			EmbreeScene* accelerator;
			uint32_t vertexCount = 0;
#if USE_EMBREE_GEOMETRY == 1
			RTCGeometry rtcGeometry = nullptr;
#endif
			size_t estimatedMemoryUsage()const;
			Mesh(const std::string& filename);

			static std::shared_ptr<Mesh> instantiate(std::shared_ptr<Mesh>parent,
				const std::string& name, const Transform& transform = Transform());

			void resetTransform(const Transform& T);

			std::unordered_map<const Primitive*, Light*> lightMap;

			void clearLightSetup() {
				for (auto& p : primitives) {
					p.setLight(nullptr);
				}
				lightMap.clear();
			}
			void release();
			void releaseVerticesWhenAddedToAccelerator();
			~Mesh();
			std::vector<Vec3f>& getVerticies() {
				Assert(loaded);
				return vertices;
			}
			const std::vector<Vec3f>& getVerticies()const {
				Assert(loaded);
				return vertices;
			}
			std::vector<Vec3f>& getNormals() {
				Assert(loaded);
				return normals;
			}
			const std::vector<Vec3f>& getNormals()const {
				Assert(loaded);
				return normals;
			}
			std::vector<Primitive>& getPrimitives() {
				Assert(loaded);
				return primitives;
			}
			const std::vector<Primitive>& getPrimitives()const {
				Assert(loaded);
				return primitives;
			}
			const std::vector<Point3i>& getVertexIndicies() const {
				Assert(loaded);
				return vertexIndices;
			}
			std::vector<Point3i>& getVertexIndicies() {
				Assert(loaded);
				return vertexIndices;
			}
			const std::vector<Point3i>& getNormalIndicies() const {
				Assert(loaded);
				return normalIndices;
			}
			std::vector<Point3i>& getNormalIndicies() {
				Assert(loaded);
				return normalIndices;
			}
			void reload();
			bool isLoaded()const {
				return loaded;
			}
		private:
			void load(const std::string& filename);
			std::vector<Vec3f> vertices, normals;
			std::vector<Primitive> primitives;
			std::vector<Point3i> vertexIndices, normalIndices;
			bool loaded = false;
			bool addedToAccelerator = false;
		};

		inline Vec3f Primitive::v(int32_t i) const {
#if USE_EMBREE_GEOMETRY == 1
			auto vertices = (Float*)rtcGetGeometryBufferData(instance->rtcGeometry, RTC_BUFFER_TYPE_VERTEX, 0);
			auto indicies = (uint32_t*)rtcGetGeometryBufferData(instance->rtcGeometry,
				RTC_BUFFER_TYPE_INDEX, 0);
			return Vec3f{ vertices[indicies[3 * primitiveId + i] * 3 + 0],
						 vertices[indicies[3 * primitiveId + i] * 3 + 1],
						 vertices[indicies[3 * primitiveId + i] * 3 + 2] };
#else
			return instance->vertices[vertices[i]];
#endif
			}

		inline const Vec3f& Primitive::n(int32_t i) const {
			return instance->normals[instance->normalIndices[primitiveId][i]];
		}

		inline Vec3f Primitive::Ns(const Point2f& uv) const {
			return PointOnTriangle(n(0), n(1), n(2), uv[0], uv[1]).normalized();
		}

		inline Material* Primitive::material() const {
			return instance->materials[nameId];
		}

		inline const std::string& Primitive::name() const {
			return instance->names[nameId];
		}

		inline bool Primitive::intersect(const Ray& ray, Intersection* isct) const {
			const Float eps = 0.00001f;
			Vec3f edge1, edge2, h, s, q;
			Float a, f, u, v;
			edge1 = this->v(1) - this->v(0);
			edge2 = this->v(2) - this->v(0);
			h = Vec3f::cross(ray.d, edge2);
			a = Vec3f::dot(edge1, h);
			if (a > -eps && a < eps)
				return false;    // This ray is parallel to this triangle.
			f = 1.0f / a;
			s = ray.o - this->v(0);
			u = f * (Vec3f::dot(s, h));
			if (u < 0.0f || u > 1.0f)
				return false;
			q = Vec3f::cross(s, edge1);
			v = f * Vec3f::dot(ray.d, q);
			if (v < 0.0f || u + v > 1.0f)
				return false;
			float t = f * Vec3f::dot(edge2, q);
			if (t > eps) // ray intersection
			{
				isct->distance = t;
				isct->p = ray.o + t * ray.d;
				isct->Ng = Ng();
				isct->uv = { u, v };
				return true;
			}
			else
				return false;
		}

		inline Light* Primitive::light() const {
			auto iter = instance->lightMap.find(this);
			if (iter == instance->lightMap.end())
				return nullptr;
			return iter->second;
		}

		inline void Primitive::setLight(Light* light) {
			if (!light)return;
			instance->lightMap[this] = light;
		}


		}
	}
#endif