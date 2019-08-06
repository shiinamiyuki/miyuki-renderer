// AUTO GENERATED. DO NOT EDIT
#ifndef MIYUKI_KERNEL_SHADER_GENERATED_H
#define MIYUKI_KERNEL_SHADER_GENERATED_H

#include "kerneldef.h"

MYK_KERNEL_NS_BEGIN
enum ShaderType{
    ESHADER_NONE,
    EFLOAT_SHADER,
    EFLOAT3_SHADER,
    EIMAGE_TEXTURE_SHADER,
    EMIXED_SHADER,
    ESCALED_SHADER,
};
struct Shader;
struct FloatShader;
struct Float3Shader;
struct ImageTextureShader;
struct MixedShader;
struct ScaledShader;

typedef struct Shader{
    ShaderType type_tag;


}Shader;

#define DISPATCH_SHADER(method,object, ...) \
    switch(object->type_tag) {\
    case EFLOAT_SHADER:\
        return float_shader##_##method((FloatShader *)object, __VA_ARGS__);\
    case EFLOAT3_SHADER:\
        return float3_shader##_##method((Float3Shader *)object, __VA_ARGS__);\
    case EIMAGE_TEXTURE_SHADER:\
        return image_texture_shader##_##method((ImageTextureShader *)object, __VA_ARGS__);\
    case EMIXED_SHADER:\
        return mixed_shader##_##method((MixedShader *)object, __VA_ARGS__);\
    case ESCALED_SHADER:\
        return scaled_shader##_##method((ScaledShader *)object, __VA_ARGS__);\
    };\
    assert(0);

void create_shader(Shader* object){
}

typedef struct FloatShader{
    Shader _base;
    float value;

}FloatShader;

void create_float_shader(FloatShader* object){
    create_shader((Shader *)object);
    object->_base.type_tag = EFLOAT_SHADER;
}

typedef struct Float3Shader{
    Shader _base;
    float3 value;
    float multiplier;

}Float3Shader;

void create_float3_shader(Float3Shader* object){
    create_shader((Shader *)object);
    object->_base.type_tag = EFLOAT3_SHADER;
}

typedef struct ImageTextureShader{
    Shader _base;
    ImageTexture * texture;

}ImageTextureShader;

void create_image_texture_shader(ImageTextureShader* object){
    object->texture = NULL;
    create_shader((Shader *)object);
    object->_base.type_tag = EIMAGE_TEXTURE_SHADER;
}

typedef struct MixedShader{
    Shader _base;
    Shader * fraction;
    Shader * shaderA;
    Shader * shaderB;

}MixedShader;

void create_mixed_shader(MixedShader* object){
    object->fraction = NULL;
    object->shaderA = NULL;
    object->shaderB = NULL;
    create_shader((Shader *)object);
    object->_base.type_tag = EMIXED_SHADER;
}

typedef struct ScaledShader{
    Shader _base;
    Shader * scale;
    Shader * shader;

}ScaledShader;

void create_scaled_shader(ScaledShader* object){
    object->scale = NULL;
    object->shader = NULL;
    create_shader((Shader *)object);
    object->_base.type_tag = ESCALED_SHADER;
}

MYK_KERNEL_NS_END

#endif
