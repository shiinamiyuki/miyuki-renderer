//
// Created by Shiina Miyuki on 2019/2/10.
//

#include "obj2json.h"

#include "../utils/jsonparser.hpp"

using namespace Miyuki;

void processFile(const std::string &filename) {
    readUnderPath(filename, [&](const std::string &file) {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string err;
        fmt::print("Loading OBJ file {}\n", filename);
        bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, file.c_str());

        if (!err.empty()) { // `err` may contain warning message.
            fmt::print(stderr, "{}\n", err);
        }
        if (!ret) {
            fmt::print(stderr, "error loading OBJ file {}\n", file);
            return;
        }
        for (const auto &m :materials) {

        }

// Loop over shapes
        for (size_t s = 0; s < shapes.size(); s++) {
            // Loop over faces(polygon)
            size_t index_offset = 0;
            for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
                int32_t fv = shapes[s].mesh.num_face_vertices[f];
                assert(fv == 3); // we are using trig mesh
                // Loop over vertices in the face.
                for (size_t v = 0; v < fv; v++) {
                    // access to vertex
                    tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

//                tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index + 0];
//                tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
//                tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];
//                tinyobj::real_t nx = attrib.normals[3 * idx.normal_index + 0];
//                tinyobj::real_t ny = attrib.normals[3 * idx.normal_index + 1];
//                tinyobj::real_t nz = attrib.normals[3 * idx.normal_index + 2];
//                tinyobj::real_t tx = attrib.texcoords[2 * idx.texcoord_index + 0];
//                tinyobj::real_t ty = attrib.texcoords[2 * idx.texcoord_index + 1];
                    // Optional: vertex colors
                    // tinyobj::real_t red = attrib.colors[3*idx.vertex_index+0];
                    // tinyobj::real_t green = attrib.colors[3*idx.vertex_index+1];
                    // tinyobj::real_t blue = attrib.colors[3*idx.vertex_index+2];
                }

                index_offset += fv;
            }
        }
    });
}

#include <iostream>

using namespace Miyuki::Json;


int main() {

}

