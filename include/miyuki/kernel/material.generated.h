// AUTO GENERATED. DO NOT EDIT
#ifndef MIYUKI_KERNEL_MATERIAL_GENERATED_H
#define MIYUKI_KERNEL_MATERIAL_GENERATED_H

#include "kerneldef.h"
#include "shader.h"
#include "bsdflobe.h"

MYK_KERNEL_NS_BEGIN
enum MaterialType{
    MATERIAL_NONE,
    DIFFUSE_MATERIAL,
    GLOSSY_MATERIAL,
    MIXED_MATERIAL,
};
struct Material;
struct DiffuseMaterial;
struct GlossyMaterial;
struct MixedMaterial;

typedef struct Material{
    MaterialType type_tag;
    BSDFLobe lobe;

}Material;

#define DISPATCH_MATERIAL(method,object, ...) \
    switch(object->type_tag) {\
    case DIFFUSE_MATERIAL:\
        return diffuse_material##_##method((DiffuseMaterial *)object, __VA_ARGS__);\
    case GLOSSY_MATERIAL:\
        return glossy_material##_##method((GlossyMaterial *)object, __VA_ARGS__);\
    case MIXED_MATERIAL:\
        return mixed_material##_##method((MixedMaterial *)object, __VA_ARGS__);\
    };\
    assert(0);

MYK_KERNEL_FUNC_INLINE void create_material(Material* object){
}

typedef struct DiffuseMaterial{
    Material _base;
    Shader * roughness;
    Shader * color;

}DiffuseMaterial;

MYK_KERNEL_FUNC_INLINE void create_diffuse_material(DiffuseMaterial* object){
    object->roughness = NULL;
    object->color = NULL;
    create_material((Material *)object);
    object->_base.type_tag = DIFFUSE_MATERIAL;
}

typedef struct GlossyMaterial{
    Material _base;
    Shader * roughness;
    Shader * color;

}GlossyMaterial;

MYK_KERNEL_FUNC_INLINE void create_glossy_material(GlossyMaterial* object){
    object->roughness = NULL;
    object->color = NULL;
    create_material((Material *)object);
    object->_base.type_tag = GLOSSY_MATERIAL;
}

typedef struct MixedMaterial{
    Material _base;
    Shader * fraction;
    Material * matA;
    Material * matB;

}MixedMaterial;

MYK_KERNEL_FUNC_INLINE void create_mixed_material(MixedMaterial* object){
    object->fraction = NULL;
    object->matA = NULL;
    object->matB = NULL;
    create_material((Material *)object);
    object->_base.type_tag = MIXED_MATERIAL;
}

MYK_KERNEL_NS_END

#endif
