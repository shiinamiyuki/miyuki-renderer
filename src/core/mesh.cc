#include <core/mesh.h>
#include <io/io.h>
#include <core/profile.h>
#include <core/ray.h>
#include <core/accelerators/accelerator.h>
#include <core/embreescene.h>
#include <utils/log.h>

namespace Miyuki {
	namespace Core {
		Mesh::Mesh(const std::string& filename) {
			Profiler profiler;
			IO::readUnderPath(filename, [&](const std::string& file) {
				name = file;
				tinyobj::attrib_t attrib;
				std::vector<tinyobj::shape_t> shapes;
				std::vector<tinyobj::material_t> materials;
				std::string err;
				Log::log("Loading OBJ file {}\n", filename);
				bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, file.c_str());

				if (!err.empty()) { // `err` may contain warning message.
					Log::log(Log::error, "{}\n", err);
				}
				if (!ret) {
					Log::log(Log::error, "error loading OBJ file {}\n", file);
					return;
				}
				CHECK(attrib.vertices.size() % 3 == 0);
				vertices.reserve(attrib.vertices.size() / 3);
				for (auto i = 0; i < attrib.vertices.size(); i += 3) {
					vertices.emplace_back(Vec3f(attrib.vertices[i],
						attrib.vertices[i + 1],
						attrib.vertices[i + 2]));
				}
				CHECK(attrib.normals.size() % 3 == 0);
				normals.reserve(attrib.normals.size() / 3);
				for (auto i = 0; i < attrib.normals.size(); i += 3) {
					normals.emplace_back(Vec3f(attrib.normals[i],
						attrib.normals[i + 1],
						attrib.normals[i + 2]));
				}
				std::unordered_map<std::string, int> map;
				for (const auto& shape : shapes) {
					auto iter = map.find(shape.name);
					if (iter == map.end()) {
						map[shape.name] = map.size();
						names.emplace_back(shape.name);
					}
				}
				CHECK(names.size() == map.size());
				for (size_t s = 0; s < shapes.size(); s++) {
					// Loop over faces(polygon)
					size_t index_offset = 0;
					for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
						int32_t fv = shapes[s].mesh.num_face_vertices[f];
						assert(fv == 3); // we are using trig mesh
						// Loop over vertices in the face.
						Primitive primitive;
						bool useNorm = true;
						for (size_t v = 0; v < fv; v++) {
							// access to vertex
							tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
							assert(idx.vertex_index < vertices.size());
							primitive.vertices[v] = idx.vertex_index;

							if (idx.normal_index < normals.size())
								primitive.normals[v] = idx.normal_index;
							else {
								useNorm = false;

							}
							if (2 * idx.texcoord_index < attrib.texcoords.size()) {
								tinyobj::real_t tx = attrib.texcoords[2 * idx.texcoord_index + 0];
								tinyobj::real_t ty = attrib.texcoords[2 * idx.texcoord_index + 1];
								primitive.textureCoord[v] = Point2f(tx, ty);
							}
							else {
								primitive.textureCoord[v] = Point2f(v > 0, v > 1);
							}
						}
						auto Ng = Vec3f::cross(vertices[primitive.vertices[1]] - vertices[primitive.vertices[0]],
							vertices[primitive.vertices[2]] - vertices[primitive.vertices[0]]);
						Ng.normalize();
						if (!useNorm) {
							normals.emplace_back(Ng);
							for (int32_t i = 0; i < 3; i++) {
								primitive.normals[i] = normals.size() - 1;
							}
						}

						primitive.nameId = map[shapes[s].name];
						primitives.emplace_back(primitive);
						index_offset += fv;
					}
				}
			});
			vertexCount = vertices.size();
			Log::log("Loaded {} in {}s, {} vertices, {} normals, {} triangles\n", filename,
				profiler.elapsedSeconds(), vertices.size(), normals.size(), primitives.size());
		}

		std::shared_ptr<Mesh> Mesh::instantiate(const std::string& name, const Transform& transform) const {
			auto mesh = std::make_shared<Mesh>(*this);
			mesh->name = name;
			mesh->transform = transform;
			for (auto& v : mesh->vertices) {
				v = transform.apply(v);
			}
			for (auto& n : mesh->normals) {
				n = transform.applyRotation(n).normalized();
			}
			for (auto& p : mesh->primitives) {
				p.instance = mesh.get();
			}
			return mesh;
		}

		void Mesh::resetTransform(const Transform& T) {
			Assert(accelerator && geomId != -1 && rtcGeometry);
			rtcDetachGeometry(accelerator->getRTCScene(), geomId);
			auto vertices = (Float*)rtcGetGeometryBufferData(rtcGeometry, RTC_BUFFER_TYPE_VERTEX, 0);
			for (int32_t i = 0; i < vertexCount; i++) {
				auto v = Vec3f(vertices[3 * i], vertices[3 * i + 1], vertices[3 * i + 2]);
				v = transform.apply(v, true);
				v = T.apply(v);
				for (int32_t j = 0; j < 3; j++)
					vertices[3 * i + j] = v[j];
			}

			for (auto& i : normals) {
				i = transform.applyRotation(i, true);
				i = T.applyRotation(i).normalized();
			}
			transform = T;

			rtcCommitGeometry(rtcGeometry);
			rtcAttachGeometryByID(accelerator->getRTCScene(), rtcGeometry, geomId);
			rtcCommitScene(accelerator->getRTCScene());
		}

		Mesh::~Mesh() {
			if (rtcGeometry) {
				rtcReleaseGeometry(rtcGeometry);
			}

		}

		Primitive::Primitive() : instance(nullptr), nameId(-1) {

		}

		Vec3f Primitive::v(int32_t i) const {
#if USE_EMBREE_GEOMETRY == 1
			auto vertices = (Float*)rtcGetGeometryBufferData(instance->rtcGeometry, RTC_BUFFER_TYPE_VERTEX, 0);
			return Vec3f{ vertices[this->vertices[i] * 3],
						 vertices[this->vertices[i] * 3 + 1],
						 vertices[this->vertices[i] * 3 + 2] };
#else
			return instance->vertices[vertices[i]];
#endif
		}

		const Vec3f& Primitive::n(int32_t i) const {
			return instance->normals[normals[i]];
		}

		Vec3f Primitive::Ns(const Point2f& uv) const {
			return PointOnTriangle(n(0), n(1), n(2), uv[0], uv[1]).normalized();
		}

		Material* Primitive::material() const {
			return instance->materials[nameId].get();
		}

		const std::string& Primitive::name() const {
			return instance->names[nameId];
		}

		bool Primitive::intersect(const Ray& ray, Intersection* isct) const {
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
				return true;
			}
			else
				return false;
		}

		Light* Primitive::light() const {
			auto iter = instance->lightMap.find(this);
			if (iter == instance->lightMap.end())
				return nullptr;
			return iter->second;
		}

		void Primitive::setLight(Light* light) {
			instance->lightMap[this] = light;
		}
	}
}