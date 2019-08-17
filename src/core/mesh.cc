#include <core/mesh.h>
#include <io/io.h>
#include <core/profile.h>
#include <core/ray.h>
#include <core/accelerators/accelerator.h>
#include <core/embreescene.h>
#include <utils/log.h>

namespace Miyuki {
	namespace Core {
		void Mesh::load(const std::string& filename) {
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
				for (size_t i = 0; i < attrib.vertices.size(); i += 3) {
					vertices.emplace_back(Vec3f(attrib.vertices[i],
						attrib.vertices[i + 1ull],
						attrib.vertices[i + 2ull]));
				}
				CHECK(attrib.normals.size() % 3 == 0);
				normals.reserve(attrib.normals.size() / 3);
				for (size_t i = 0; i < attrib.normals.size(); i += 3) {
					normals.emplace_back(Vec3f(attrib.normals[i],
						attrib.normals[i + 1ull],
						attrib.normals[i + 2ull]));
				}
				std::unordered_map<std::string, int> map;
				for (const auto& shape : shapes) {
					auto iter = map.find(shape.name);
					if (iter == map.end()) {
						map[shape.name] = (int)map.size();
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
						Point3i pvertices, pnormals;
						bool useNorm = true;
						for (size_t v = 0; v < fv; v++) {
							// access to vertex
							tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
							assert(idx.vertex_index < vertices.size());

							pvertices[v] = idx.vertex_index;

							if (idx.normal_index < normals.size())
								pnormals[v] = idx.normal_index;
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
						auto Ng = Vec3f::cross(vertices[pvertices[1]] - vertices[pvertices[0]],
							vertices[pvertices[2]] - vertices[pvertices[0]]);
						Ng.normalize();
						if (!useNorm) {
							normals.emplace_back(Ng);
							for (int32_t i = 0; i < 3; i++) {
								pnormals[i] = (uint32_t)normals.size() - 1;
							}
						}
						primitive.primitiveId = primitives.size();
						primitive.nameId = map[shapes[s].name];
						primitives.emplace_back(primitive);
						vertexIndices.emplace_back(pvertices);
						normalIndices.emplace_back(pnormals);
						index_offset += fv;
					}
				}
			});
			vertexCount = vertices.size();
			Log::log("Loaded {} in {}s, {} vertices, {} normals, {} triangles\n", filename,
				profiler.elapsedSeconds(), vertices.size(), normals.size(), primitives.size());
			loaded = true;
			addedToAccelerator = false;
		}
		Mesh::Mesh(const std::string& filename) :filename(filename) {
			load(filename);
		}
		void Mesh::reload() {
			load(filename);
		}
		size_t Mesh::estimatedMemoryUsage()const {
			return sizeof(Mesh) + (vertices.capacity() + normals.capacity()) * sizeof(Vec3f)
				+ sizeof(Point3i) * (vertexIndices.capacity() + normalIndices.capacity())
				+ sizeof(Primitive) * primitives.capacity()
				+ lightMap.size() * sizeof(std::pair<const Primitive*, Light*>);
		}
		void Mesh::release() {
			loaded = false;
			vertices = std::vector<Vec3f>();
			normals = std::vector<Vec3f>();
			primitives = std::vector<Primitive>();
			vertexIndices = std::vector<Point3i>();
			normalIndices = std::vector<Point3i>();
			lightMap.clear();
		}
		void Mesh::releaseVerticesWhenAddedToAccelerator() {
			addedToAccelerator = true;
			vertices = std::vector<Vec3f>();
			vertexIndices = std::vector<Point3i>();
		}
		std::shared_ptr<Mesh> Mesh::instantiate(std::shared_ptr<Mesh>parent,
			const std::string& name, const Transform& transform) {
			Assert(parent->loaded);
			auto mesh = std::make_shared<Mesh>(*parent);
			mesh->name = name;
			mesh->loaded = true;
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
	}
}