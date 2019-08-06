// AUTO GENERATED. DO NOT EDIT
#ifndef MIYUKI_KERNEL_MATERIAL_GENERATED_H
#define MIYUKI_KERNEL_MATERIAL_GENERATED_H

#include "kerneldef.h"
#include "shader.h"

MYK_KERNEL_NS_BEGIN
enum MaterialType{
    EMATERIAL_NONE,
    EDIFFUSE_MATERIAL,
    EGLOSSY_MATERIAL,
    EMIXED_MATERIAL,
};
struct Material;
struct DiffuseMaterial;
struct GlossyMaterial;
struct MixedMaterial;

typedef struct Material{
    MaterialType type_tag;


}Material;

#define DISPATCH_MATERIAL(method,object, ...) \
    switch(object->type_tag) {\
    case EDIFFUSE_MATERIAL:\
        return diffuse_material##_##method((DiffuseMaterial *)object, __VA_ARGS__);\
    case EGLOSSY_MATERIAL:\
        return glossy_material##_##method((GlossyMaterial *)object, __VA_ARGS__);\
    case EMIXED_MATERIAL:\
        return mixed_material##_##method((MixedMaterial *)object, __VA_ARGS__);\
    };\
    assert(0);

void create_material(Material* object){
}

typedef struct DiffuseMaterial{
    Material _base;
    Shader * roughness;
    Shader * color;

}DiffuseMaterial;

void create_diffuse_material(DiffuseMaterial* object){
    object->roughness = NULL;
    object->color = NULL;
    create_material((Material *)object);
    object->_base.type_tag = EDIFFUSE_MATERIAL;
}

typedef struct GlossyMaterial{
    Material _base;
    Shader * roughness;
    Shader * color;

}GlossyMaterial;

void create_glossy_material(GlossyMaterial* object){
    object->roughness = NULL;
    object->color = NULL;
    create_material((Material *)object);
    object->_base.type_tag = EGLOSSY_MATERIAL;
}

typedef struct MixedMaterial{
    Material _base;
    Shader * fraction;
    Material * matA;
    Material * matB;

}MixedMaterial;

void create_mixed_material(MixedMaterial* object){
    object->fraction = NULL;
    object->matA = NULL;
    object->matB = NULL;
    create_material((Material *)object);
    object->_base.type_tag = EMIXED_MATERIAL;
}

MYK_KERNEL_NS_END

#endif
