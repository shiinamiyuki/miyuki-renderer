#pragma once
#include <api/defs.h>
#include <api/material.h>
#include <api/object.hpp>
namespace miyuki ::core {
    class Mesh;

    struct MeshImportResult {
        std::vector<Material> materials;
        std::shared_ptr<Mesh> mesh;
    };
    class MeshImporter : public Object {
      public:
        virtual MeshImportResult importMesh(const fs::path &) = 0;
    };
} // namespace miyuki::core