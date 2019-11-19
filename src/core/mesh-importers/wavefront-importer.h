#pragma once

#include <api/mesh-importer.h>
#include <api/serialize.hpp>

namespace miyuki::core {

    class WavefrontImporter final : public MeshImporter {
      public:
        MYK_DECL_CLASS(WavefrontImporter, "WavefrontImporter", interface = "MeshImporter")
        MeshImportResult importMesh(const fs::path &) override;
    };
} // namespace miyuki::core