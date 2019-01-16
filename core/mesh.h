//
// Created by xiaoc on 2019/1/15.
//

#ifndef MIYUKI_MESH_H
#define MIYUKI_MESH_H

#include "geometry.h"
#include "scene.h"

namespace Miyuki {
    namespace Mesh {
        struct Triangle {
            int vertex[3];
            int norm[3];
            int materialId;
            bool useNorm;
            Vec3f trigNorm;
            Float area;
            Triangle()= default;
        };
        class TriangularMesh;
        std::shared_ptr<TriangularMesh> LoadFromObj(MaterialList *materialList, const char *filename);
        class TriangularMesh {
            std::vector<Vec3f> vertex, norm;
            std::vector<Triangle> trigs;
        public:
            TriangularMesh() {}
            friend std::shared_ptr<TriangularMesh> LoadFromObj(MaterialList *materialList, const char *filename);
        };


    }
}
#endif //MIYUKI_MESH_H
