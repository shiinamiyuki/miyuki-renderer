/*
The MIT License (MIT)

Copyright (c) 2012-2016 Syoyo Fujita and many contributors.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

//
// version 1.0.6 : Add TINYOBJLOADER_USE_DOUBLE option(#124)
// version 1.0.5 : Ignore `Tr` when `d` exists in MTL(#43)
// version 1.0.4 : Support multiple filenames for 'mtllib'(#112)
// version 1.0.3 : Support parsing texture options(#85)
// version 1.0.2 : Improve parsing speed by about a factor of 2 for large
// files(#105)
// version 1.0.1 : Fixes a shape is lost if obj ends with a 'usemtl'(#104)
// version 1.0.0 : Change data structure. Change license from BSD to MIT.
//

//
// Use this in *one* .cc
//   #define TINYOBJLOADER_IMPLEMENTATION
//   #include "tiny_obj_loader.h"
//

#ifndef TINY_OBJ_LOADER_H_
#define TINY_OBJ_LOADER_H_

#include <map>
#include <string>
#include <vector>

namespace tinyobj {

// https://en.wikipedia.org/wiki/Wavefront_.obj_file says ...
//
//  -blendu on | off                       # set horizontal texture blending
//  (default on)
//  -blendv on | off                       # set vertical texture blending
//  (default on)
//  -boost real_value                      # boost mip-map sharpness
//  -mm base_value gain_value              # modify texture map values (default
//  0 1)
//                                         #     base_value = brightness,
//                                         gain_value = contrast
//  -o u [v [w]]                           # Origin offset             (default
//  0 0 0)
//  -s u [v [w]]                           # Scale                     (default
//  1 1 1)
//  -t u [v [w]]                           # Turbulence                (default
//  0 0 0)
//  -texres resolution                     # texture resolution to create
//  -clamp on | off                        # only render texels in the clamped
//  0-1 range (default off)
//                                         #   When unclamped, textures are
//                                         repeated across a surface,
//                                         #   when clamped, only texels which
//                                         fall within the 0-1
//                                         #   range are rendered.
//  -bm mult_value                         # bump multiplier (for bump maps
//  only)
//
//  -imfchan r | g | b | m | l | z         # specifies which channel of the file
//  is used to
//                                         # create a scalar or bump texture.
//                                         r:red, g:green,
//                                         # b:blue, m:matte, l:luminance,
//                                         z:z-depth..
//                                         # (the default for bump is 'l' and
//                                         for decal is 'm')
//  bump -imfchan r bumpmap.tga            # says to use the red channel of
//  bumpmap.tga as the bumpmap
//
// For reflection maps...
//
//   -type sphere                           # specifies a sphere for a "refl"
//   reflection map
//   -type cube_top    | cube_bottom |      # when using a cube map, the texture
//   file for each
//         cube_front  | cube_back   |      # side of the cube is specified
//         separately
//         cube_left   | cube_right

#ifdef TINYOBJLOADER_USE_DOUBLE
  //#pragma message "using double"
  typedef double real_t;
#else
  //#pragma message "using float"
  typedef float real_t;
#endif

typedef enum {
  TEXTURE_TYPE_NONE,  // default
  TEXTURE_TYPE_SPHERE,
  TEXTURE_TYPE_CUBE_TOP,
  TEXTURE_TYPE_CUBE_BOTTOM,
  TEXTURE_TYPE_CUBE_FRONT,
  TEXTURE_TYPE_CUBE_BACK,
  TEXTURE_TYPE_CUBE_LEFT,
  TEXTURE_TYPE_CUBE_RIGHT
} texture_type_t;

typedef struct {
  texture_type_t type;     // -type (default TEXTURE_TYPE_NONE)
  real_t sharpness;         // -boost (default 1.0?)
  real_t brightness;        // base_value in -mm option (default 0)
  real_t contrast;          // gain_value in -mm option (default 1)
  real_t origin_offset[3];  // -o u [v [w]] (default 0 0 0)
  real_t scale[3];          // -s u [v [w]] (default 1 1 1)
  real_t turbulence[3];     // -t u [v [w]] (default 0 0 0)
  // int   texture_resolution; // -texres resolution (default = ?) TODO
  bool clamp;    // -clamp (default false)
  char imfchan;  // -imfchan (the default for bump is 'l' and for decal is 'm')
  bool blendu;   // -blendu (default on)
  bool blendv;   // -blendv (default on)
  real_t bump_multiplier;  // -bm (for bump maps only, default 1.0)
} texture_option_t;

typedef struct {
  std::string name;
  std::string bsdfType;
  real_t ambient[3];
  real_t diffuse[3];
  real_t specular[3];
  real_t transmittance[3];
  real_t emission[3];
  real_t shininess;
  real_t ior;       // index of refraction
  real_t dissolve;  // 1 == opaque; 0 == fully transparent
  // illumination model (see http://www.fileformat.info/format/material/)
  int illum;

  int dummy;  // Suppress padding warning.

  std::string ambient_texname;             // map_Ka
  std::string diffuse_texname;             // map_Kd
  std::string specular_texname;            // map_Ks
  std::string specular_highlight_texname;  // map_Ns
  std::string bump_texname;                // map_bump, bump
  std::string displacement_texname;        // disp
  std::string alpha_texname;               // map_d

  texture_option_t ambient_texopt;
  texture_option_t diffuse_texopt;
  texture_option_t specular_texopt;
  texture_option_t specular_highlight_texopt;
  texture_option_t bump_texopt;
  texture_option_t displacement_texopt;
  texture_option_t alpha_texopt;

  // PBR extension
  // http://exocortex.com/blog/extending_wavefront_mtl_to_support_pbr
  real_t roughness;            // [0, 1] default 0
  real_t metallic;             // [0, 1] default 0
  real_t sheen;                // [0, 1] default 0
  real_t clearcoat_thickness;  // [0, 1] default 0
  real_t clearcoat_roughness;  // [0, 1] default 0
  real_t anisotropy;           // aniso. [0, 1] default 0
  real_t anisotropy_rotation;  // anisor. [0, 1] default 0
  real_t pad0;
  real_t pad1;
  std::string roughness_texname;  // map_Pr
  std::string metallic_texname;   // map_Pm
  std::string sheen_texname;      // map_Ps
  std::string emissive_texname;   // map_Ke
  std::string normal_texname;     // norm. For normal mapping.

  texture_option_t roughness_texopt;
  texture_option_t metallic_texopt;
  texture_option_t sheen_texopt;
  texture_option_t emissive_texopt;
  texture_option_t normal_texopt;

  int pad2;

  std::map<std::string, std::string> unknown_parameter;
} material_t;

typedef struct {
  std::string name;

  std::vector<int> intValues;
  std::vector<real_t> floatValues;
  std::vector<std::string> stringValues;
} tag_t;

// Index struct to support different indices for vtx/normal/texcoord.
// -1 means not used.
typedef struct {
  int vertex_index;
  int normal_index;
  int texcoord_index;
} index_t;

typedef struct {
  std::vector<index_t> indices;
  std::vector<unsigned char> num_face_vertices;  // The number of vertices per
                                                 // face. 3 = polygon, 4 = quad,
                                                 // ... Up to 255.
  std::vector<int> material_ids;                 // per-face material ID
  std::vector<tag_t> tags;                       // SubD tag
} mesh_t;

typedef struct {
  std::string name;
  mesh_t mesh;
} shape_t;

// Vertex attributes
typedef struct {
  std::vector<real_t> vertices;   // 'v'
  std::vector<real_t> normals;    // 'vn'
  std::vector<real_t> texcoords;  // 'vt'
} attrib_t;

typedef struct callback_t_ {
  // W is optional and set to 1 if there is no `w` item in `v` line
  void (*vertex_cb)(void *user_data, real_t x, real_t y, real_t z, real_t w);
  void (*normal_cb)(void *user_data, real_t x, real_t y, real_t z);

  // y and z are optional and set to 0 if there is no `y` and/or `z` item(s) in
  // `vt` line.
  void (*texcoord_cb)(void *user_data, real_t x, real_t y, real_t z);

  // called per 'f' line. num_indices is the number of face indices(e.g. 3 for
  // triangle, 4 for quad)
  // 0 will be passed for undefined index in index_t members.
  void (*index_cb)(void *user_data, index_t *indices, int num_indices);
  // `name` material name, `material_id` = the array index of material_t[]. -1
  // if
  // a material not found in .mtl
  void (*usemtl_cb)(void *user_data, const char *name, int material_id);
  // `materials` = parsed material data.
  void (*mtllib_cb)(void *user_data, const material_t *materials,
                    int num_materials);
  // There may be multiple group names
  void (*group_cb)(void *user_data, const char **names, int num_names);
  void (*object_cb)(void *user_data, const char *name);

  callback_t_()
      : vertex_cb(NULL),
        normal_cb(NULL),
        texcoord_cb(NULL),
        index_cb(NULL),
        usemtl_cb(NULL),
        mtllib_cb(NULL),
        group_cb(NULL),
        object_cb(NULL) {}
} callback_t;

class MaterialReader {
 public:
  MaterialReader() {}
  virtual ~MaterialReader();

  virtual bool operator()(const std::string &matId,
                          std::vector<material_t> *materials,
                          std::map<std::string, int> *matMap,
                          std::string *err) = 0;
};

class MaterialFileReader : public MaterialReader {
 public:
  explicit MaterialFileReader(const std::string &mtl_basedir)
      : m_mtlBaseDir(mtl_basedir) {}
  virtual ~MaterialFileReader() {}
  virtual bool operator()(const std::string &matId,
                          std::vector<material_t> *materials,
                          std::map<std::string, int> *matMap, std::string *err);

 private:
  std::string m_mtlBaseDir;
};

class MaterialStreamReader : public MaterialReader {
 public:
  explicit MaterialStreamReader(std::istream &inStream)
      : m_inStream(inStream) {}
  virtual ~MaterialStreamReader() {}
  virtual bool operator()(const std::string &matId,
                          std::vector<material_t> *materials,
                          std::map<std::string, int> *matMap, std::string *err);

 private:
  std::istream &m_inStream;
};

/// Loads .obj from a file.
/// 'attrib', 'shapes' and 'materials' will be filled with parsed shape data
/// 'shapes' will be filled with parsed shape data
/// Returns true when loading .obj become success.
/// Returns warning and error message into `err`
/// 'mtl_basedir' is optional, and used for base directory for .mtl file.
/// In default(`NULL'), .mtl file is searched from an application's working
/// directory.
/// 'triangulate' is optional, and used whether triangulate polygon face in .obj
/// or not.
bool LoadObj(attrib_t *attrib, std::vector<shape_t> *shapes,
             std::vector<material_t> *materials, std::string *err,
             const char *filename, const char *mtl_basedir = NULL,
             bool triangulate = true);

/// Loads .obj from a file with custom user callback.
/// .mtl is loaded as usual and parsed material_t data will be passed to
/// `callback.mtllib_cb`.
/// Returns true when loading .obj/.mtl become success.
/// Returns warning and error message into `err`
/// See `examples/callback_api/` for how to use this function.
bool LoadObjWithCallback(std::istream &inStream, const callback_t &callback,
                         void *user_data = NULL,
                         MaterialReader *readMatFn = NULL,
                         std::string *err = NULL);

/// Loads object from a std::istream, uses GetMtlIStreamFn to retrieve
/// std::istream for materials.
/// Returns true when loading .obj become success.
/// Returns warning and error message into `err`
bool LoadObj(attrib_t *attrib, std::vector<shape_t> *shapes,
             std::vector<material_t> *materials, std::string *err,
             std::istream *inStream, MaterialReader *readMatFn = NULL,
             bool triangulate = true);

/// Loads materials into std::map
void LoadMtl(std::map<std::string, int> *material_map,
             std::vector<material_t> *materials, std::istream *inStream,
             std::string *warning);

}  // namespace tinyobj

#endif  // TINY_OBJ_LOADER_H_


