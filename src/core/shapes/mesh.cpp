// MIT License
// 
// Copyright (c) 2019 椎名深雪
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
#include <api/mesh.h>
#include <tiny_obj_loader.h>
#include <api/log.hpp>
#include <fstream>


namespace miyuki::core {
    template<class T>
    static void write(std::vector<char> &buffer, const T &v) {
        unsigned char _buffer[sizeof(T)];
        *(T *) _buffer = v;
        for (auto i: _buffer) {
            buffer.emplace_back(i);
        }
    }

    template<class Iter, class T>
    static Iter read(Iter begin, Iter end, T &v) {
        unsigned char _buffer[sizeof(T)];
        for (auto &i: _buffer) {
            if (begin == end) {
                MIYUKI_THROW(std::out_of_range, "invalid iter in static Iter read(Iter begin, Iter end, T &v)");
            }
            i = *begin;
            begin++;

        }
        v = *(T *) _buffer;
        return begin;
    }

    static void writeString(std::vector<char> &buffer, const std::string &s) {
        write(buffer, s.length());
        for (int i = 0; i < s.length(); i++) {
            write(buffer, s[i]);
        }
    }

    template<class Iter>
    static Iter readString(Iter begin, Iter end, std::string &s) {
        size_t length;
        begin = read(begin, end, length);
        for (int i = 0; i < length; i++) {
            char c;
            begin = read(begin, end, c);
            s += c;
        }
        return begin;
    }

    void Mesh::preprocess() {
        if (!_loaded) {
            auto ext = fs::path(filename).extension().string();
            if (ext == ".mesh")
                loadFromFile(filename);
            else if (ext == ".obj") {
                importFromFile(filename);
            }
        }
        for (int i = 0; i < triangles.size(); i++) {
            triangles[i].primID = i;
        }
        _materials.clear();
        for (const auto &name : _names) {
            if(materials.find(name)!=materials.end()) {
                auto mat = materials.at(name);
                _materials.emplace_back(mat);
            }else{
                _materials.emplace_back(nullptr);
            }
        }
        accelerator = std::dynamic_pointer_cast<Accelerator>(CreateEntity("BVHAccelerator"));
        accelerator->build(this);
    }

    bool Mesh::importFromFile(const std::string &filename) {
        fs::path path(filename);
        fs::path parent_path = path.parent_path();
        fs::path file = path.filename();
        CurrentPathGuard _guard;
        if (!parent_path.empty())
            fs::current_path(parent_path);

        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;

        std::string warn;
        std::string err;

        std::string _file = file.string();
        bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, _file.c_str());
        if (!ret) {
            log::log("error loading {}\n", filename);
            return false;
        }

        auto mesh = this;

        mesh->_vertex_data.position.reserve(attrib.vertices.size());
        for (size_t i = 0; i < attrib.vertices.size(); i += 3) {
            mesh->_vertex_data.position.emplace_back(Vec3f(
                    attrib.vertices[i + 0],
                    attrib.vertices[i + 1],
                    attrib.vertices[i + 2]));
        }

        mesh->_vertex_data.normal.resize(mesh->_vertex_data.position.size());
        mesh->_vertex_data.tex_coord.resize(mesh->_vertex_data.position.size());


        std::unordered_map<std::string, size_t> name_to_id;
        std::unordered_map<size_t, std::string> id_to_name;
        std::unordered_map<std::string, std::unordered_map<int, std::string>> to_mangled_name;

        for (size_t i = 0; i < shapes.size(); i++) {
            const auto &name = shapes[i].name;
            auto iter = to_mangled_name.find(name);


            if (iter == to_mangled_name.end()) {
                to_mangled_name[name] = {};
            }
            auto &m = to_mangled_name[name];
            size_t count = 0;
            for (size_t f = 0; f < shapes[i].mesh.num_face_vertices.size(); f++) {
                bool new_name = false;
                std::string final_name;

                auto mat_id = shapes[i].mesh.material_ids[f];
                if (m.find(mat_id) == m.end() || mat_id == -1) {
                    new_name = true;
                    if (mat_id >= 0) {
                        final_name = fmt::format("{}:{}", name, materials[mat_id].name);
                    } else {
                        final_name = fmt::format("{}:part{}", name, count + 1);
                    }
                }
                if (new_name) {
                    count++;
                    m[mat_id] = final_name;
                    name_to_id[final_name] = name_to_id.size();
                    id_to_name[name_to_id[final_name]] = final_name;
                    fmt::print("generated {}\n", final_name);
                }
            }
        }
        for (size_t i = 0; i < name_to_id.size(); i++) {
            mesh->_names.emplace_back(id_to_name[i]);
        }

        // Loop over shapes
        for (size_t s = 0; s < shapes.size(); s++) {
            // Loop over faces(polygon)
            size_t index_offset = 0;
            for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
                MeshTriangle primitive;
                auto mat_id = shapes[s].mesh.material_ids[f];
                primitive.name_id = name_to_id.at(to_mangled_name.at(shapes[s].name).at(mat_id));
                int fv = shapes[s].mesh.num_face_vertices[f];
                Point3i vertex_indices;
                // Loop over vertices in the face.
                for (size_t v = 0; v < fv; v++) {
                    // access to vertex
                    tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                    vertex_indices[v] = idx.vertex_index;
                    if (idx.normal_index >= 0) {
                        mesh->_vertex_data.normal[idx.vertex_index] = Vec3f(
                                attrib.normals[3 * idx.normal_index + 0],
                                attrib.normals[3 * idx.normal_index + 1],
                                attrib.normals[3 * idx.normal_index + 2]
                        );
                    } else {
                        // use the last normal (Ng)

                        Vec3f e1 = mesh->_vertex_data.position[vertex_indices[2]] -
                                   mesh->_vertex_data.position[vertex_indices[0]];
                        Vec3f e2 = mesh->_vertex_data.position[vertex_indices[1]] -
                                   mesh->_vertex_data.position[vertex_indices[0]];
                        mesh->_vertex_data.normal[idx.vertex_index] = e1.cross(e2).normalized();
                    }
                    if (idx.texcoord_index >= 0) {
                        mesh->_vertex_data.tex_coord[idx.vertex_index] =
                                Point2f(attrib.texcoords[2 * idx.texcoord_index + 0],
                                        attrib.texcoords[2 * idx.texcoord_index + 1]);
                    } else {
                        mesh->_vertex_data.tex_coord[idx.vertex_index] = Point2f(v > 0, v > 1);
                    }
                }
                index_offset += fv;

                // per-face material
                shapes[s].mesh.material_ids[f];

                primitive.mesh = this;
                mesh->triangles.push_back(primitive);
                mesh->_indices.push_back(vertex_indices);
            }
        }
        log::log("loaded {} vertices, {} normals, {} primitives\n",
                 mesh->_vertex_data.position.size(), mesh->_vertex_data.normal.size(),
                 mesh->triangles.size());
        _loaded = true;
        return true;
    }

    void Mesh::writeToFile(const std::string &filename) {
        std::vector<char> buffer;
        toBinary(buffer);
        this->filename = filename;
        std::ofstream out(filename, std::ios::out | std::ios::binary);
        out.write(buffer.data(), buffer.size());
    }


    void Mesh::toBinary(std::vector<char> &buffer) const {
        writeString(buffer, "BINARY_MESH");
        write(buffer, _names.size());
        for (const auto &i:_names) {
            writeString(buffer, i);
        }
        write(buffer, _vertex_data.position.size());
        for (const auto &i: _vertex_data.position) {
            write(buffer, i);
        }
        write(buffer, _vertex_data.normal.size());
        for (const auto &i: _vertex_data.normal) {
            write(buffer, i);
        }
        write(buffer, _vertex_data.tex_coord.size());
        for (const auto &i: _vertex_data.tex_coord) {
            write(buffer, i);
        }
        write(buffer, _indices.size());
        for (const auto &i: _indices) {
            write(buffer, i);
        }
        write(buffer, triangles.size());
        for (const auto &i:triangles) {
            write(buffer, i.name_id);
        }
    }

    void Mesh::fromBinary(const std::vector<char> &buffer) {
        std::string magic;
        auto iter = buffer.begin();
        auto end = buffer.end();
        iter = readString(iter, end, magic);
        size_t name_size;
        iter = read(iter, end, name_size);
        _names.resize(name_size);
        for (auto &i:_names) {
            iter = readString(iter, end, i);
        }
        size_t size;
        iter = read(iter, end, size);
        _vertex_data.position.resize(size);
        for (auto &i: _vertex_data.position) {
            iter = read(iter, end, i);
            //fmt::print("{} {} {}\n",i.x,i.y,i.z);
        }
        iter = read(iter, end, size);
        _vertex_data.normal.resize(size);
        for (auto &i: _vertex_data.normal) {
            iter = read(iter, end, i);
        }
        iter = read(iter, end, size);
        _vertex_data.tex_coord.resize(size);
        for (auto &i: _vertex_data.tex_coord) {
            iter = read(iter, end, i);
        }
        iter = read(iter, end, size);
        _indices.resize(size);
        for (auto &i: _indices) {
            iter = read(iter, end, i);
        }
        iter = read(iter, end, size);
        triangles.resize(size);
        for (auto &i:triangles) {
            iter = read(iter, end, i.name_id);
            i.mesh = this;
        }
        MIYUKI_CHECK(_indices.size() == triangles.size());
        log::log("loaded {} vertices, {} normals, {} primitives\n",
                 _vertex_data.position.size(), _vertex_data.normal.size(),
                 triangles.size());

    }

    bool Mesh::loadFromFile(const std::string &filename) {
        try {
            std::ifstream in(filename, std::ios::binary | std::ios::in);
            if (in) {
                in.seekg(0, std::ios::end);
                size_t size = in.tellg();
                in.seekg(0, std::ios::beg);
                std::vector<char> buffer(size);
                in.read(buffer.data(), size);
                fromBinary(buffer);
                _loaded = true;
                return true;
            } else {
                return false;
            }
        } catch (std::runtime_error &error) {
            log::log("error: {}\n", error.what());
            return false;
        }
    }

    void Mesh::foreach(const std::function<void(MeshTriangle *)> &func) {
        for (auto &i :triangles) {
            func(&i);
        }
    }

    const Point3f &MeshTriangle::vertex(size_t i) const {
        return mesh->_vertex_data.position[mesh->_indices[primID][i]];
    }

    const Normal3f &MeshTriangle::normal(size_t i) const {
        return mesh->_vertex_data.normal[mesh->_indices[primID][i]];
    }

    const Point2f &MeshTriangle::texCoord(size_t i) const {
        return mesh->_vertex_data.tex_coord[mesh->_indices[primID][i]];
    }

    Material *MeshTriangle::getMaterial() const {
        return mesh->_materials[name_id].get();
    }
}

