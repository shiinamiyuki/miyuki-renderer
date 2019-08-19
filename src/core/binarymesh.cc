#include <core/binarymesh.h>

namespace Miyuki {
	namespace Core {
		const std::string identifier = "MYKBINARYMESH";
		void writeBinaryMesh(const std::shared_ptr<Mesh> mesh, IO::BinaryWriter& writer) {
			Assert(mesh->isLoaded());
			writer.write(identifier);
			writer.write(mesh->name);
			writer.write(mesh->names.size());
			for (const auto& i : mesh->names) {
				writer.write(i);
			}
			writer.write(mesh->getVerticies().size());
			for (const auto& v : mesh->getVerticies()) {
				writer.write(v[0]);
				writer.write(v[1]);
				writer.write(v[2]);
			}
			writer.write(mesh->getNormals().size());
			for (const auto& v : mesh->getNormals()) {
				writer.write(v[0]);
				writer.write(v[1]);
				writer.write(v[2]);
			}
			writer.write(mesh->getVertexIndicies().size());
			for (const auto& v : mesh->getVertexIndicies()) {
				writer.write(v[0]);
				writer.write(v[1]);
				writer.write(v[2]);
			}
			writer.write(mesh->getNormalIndicies().size());
			for (const auto& v : mesh->getNormalIndicies()) {
				writer.write(v[0]);
				writer.write(v[1]);
				writer.write(v[2]);
			}
			writer.write(mesh->getPrimitives().size());
			for (const auto& p : mesh->getPrimitives()) {
				for (int i = 0; i < 3; i++) {
					writer.write(p.textureCoord[i][0]);
					writer.write(p.textureCoord[i][1]);
				}
				writer.write(p.primitiveId);
				writer.write(p.nameId);
			}
		}
		std::shared_ptr<Mesh> readBinaryMesh(IO::BinaryReader& reader) {
			auto h = reader.readString();
			if (h != identifier) {
				throw std::runtime_error("Not a valid binary mesh file");
			}
			auto mesh = std::make_shared<Mesh>();
			mesh->name = reader.readString();
			int len = reader.read<size_t>();
			for (auto i = 0; i < len; i++) {
				mesh->names.emplace_back(reader.readString());
			}
			len = reader.read<size_t>();
			mesh->getVerticies().reserve(len);
			for (auto i = 0; i < len; i++) {
				Vec3f v;
				v[0] = reader.read<Float>();
				v[1] = reader.read<Float>();
				v[2] = reader.read<Float>();
				mesh->getVerticies().emplace_back(v);
			}

			len = reader.read<size_t>();
			mesh->getNormals().reserve(len);
			for (auto i = 0; i < len; i++) {
				Vec3f v;
				v[0] = reader.read<Float>();
				v[1] = reader.read<Float>();
				v[2] = reader.read<Float>();
				mesh->getNormals().emplace_back(v);
			}

			len = reader.read<size_t>();
			mesh->getVertexIndicies().reserve(len);
			for (auto i = 0; i < len; i++) {
				Point3i v;
				v[0] = reader.read<int32_t>();
				v[1] = reader.read<int32_t>();
				v[2] = reader.read<int32_t>();
				mesh->getVertexIndicies().emplace_back(v);
			}

			len = reader.read<size_t>();
			mesh->getNormalIndicies().reserve(len);
			for (auto i = 0; i < len; i++) {
				Point3i v;
				v[0] = reader.read<int32_t>();
				v[1] = reader.read<int32_t>();
				v[2] = reader.read<int32_t>();
				mesh->getNormalIndicies().emplace_back(v);
			}

			len = reader.read<size_t>();
			mesh->getPrimitives().reserve(len);
			for (auto i = 0; i < len; i++) {
				Primitive primitive;
				primitive.instance = mesh.get();
				for (int i = 0; i < 3; i++) {
					Point2f text;
					text[0] = reader.read<Float>();
					text[1] = reader.read<Float>();
					primitive.textureCoord[i] = text;
				}
				primitive.primitiveId = reader.read<size_t>();
				primitive.nameId = reader.read<uint16_t>();
				mesh->getPrimitives().emplace_back(primitive);
			}
			return mesh;
		}
	}
}