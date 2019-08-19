#pragma once

#include <core/mesh.h>
#include <io/binaryio.h>

namespace Miyuki {
	namespace Core {
		void writeBinaryMesh(const std::shared_ptr<Mesh> mesh, IO::BinaryWriter& writer);
		std::shared_ptr<Mesh> readBinaryMesh(IO::BinaryReader& reader);
	}
}