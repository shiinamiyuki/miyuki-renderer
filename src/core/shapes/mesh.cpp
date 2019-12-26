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
#include <miyuki.renderer/mesh.h>
#include <miyuki.foundation/log.hpp>
#include <fstream>
#include <miyuki.renderer/bsdf.h>


namespace miyuki::core {
    template<class T>
    void write(std::vector<char> &buffer, const T &v) {
        unsigned char _buffer[sizeof(T)];
        memcpy(_buffer, &v, sizeof(T));
        for (auto i: _buffer) {
            buffer.emplace_back(i);
        }
    }

    template<class Iter, class T>
    Iter read(Iter begin, Iter end, T &v) {
        unsigned char _buffer[sizeof(T)];
        for (auto &i: _buffer) {
            if (begin == end) {
                MIYUKI_THROW(std::out_of_range, "invalid iter in static Iter read(Iter begin, Iter end, T &v)");
            }
            i = *begin;
            begin++;

        }
        memcpy(&v, _buffer, sizeof(T));
        return begin;
    }

    void writeString(std::vector<char> &buffer, const std::string &s) {
        write(buffer, s.length());
        for (int i = 0; i < s.length(); i++) {
            write(buffer, s[i]);
        }
    }

    template<class Iter>
    Iter readString(Iter begin, Iter end, std::string &s) {
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
            if (ext == ".mesh") {
                if (!loadFromFile(filename)) {
                    log::log("failed to load {}\n", fs::absolute(fs::path(filename)).string());
                }
            } else {
                throw std::runtime_error("Only .mesh files are supported");
            }
        }
        _materials.clear();
        for (const auto &name : _names) {
            if (materials.find(name) != materials.end()) {
                auto mat = materials.at(name);
                _materials.emplace_back(mat);
                if (mat->emission)
                    mat->emission->preprocess();
                if (mat->emissionStrength)
                    mat->emissionStrength->preprocess();
                if (mat->bsdf)
                    mat->bsdf->preprocess();
            } else {
                _materials.emplace_back(nullptr);
            }
        }

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
        write(buffer, triangles.size());
        for (const auto &i: triangles) {
            write(buffer, i.indices);
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

        triangles.resize(size);

        for (auto &i: triangles) {
            iter = read(iter, end, i.indices);

        }
        iter = read(iter, end, size);
        MIYUKI_CHECK(size == triangles.size());
        for (auto &i:triangles) {
            iter = read(iter, end, i.name_id);
            i.mesh = this;
        }
        log::log("loaded {} vertices, {} normals, {} tex coords, {} primitives\n",
                 _vertex_data.position.size(), _vertex_data.normal.size(), _vertex_data.tex_coord.size(),
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
        return mesh->_vertex_data.position[indices.position[i]];
    }

    Normal3f MeshTriangle::normal(size_t i) const {
        auto idx = indices.normal[i];
        return idx >= 0 ? mesh->_vertex_data.normal[idx] : Ng();
    }

    Point2f MeshTriangle::texCoord(size_t i) const {
        auto idx = indices.texCoord[i];
        return idx >= 0 ? mesh->_vertex_data.tex_coord[idx] : Point2f(i > 0, i > 1);
    }

    Material *MeshTriangle::getMaterial() const {
        return mesh->_materials[name_id].get();
    }
}

