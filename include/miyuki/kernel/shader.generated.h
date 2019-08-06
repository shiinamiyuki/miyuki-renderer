// AUTO GENERATED. DO NOT EDIT
#ifndef MIYUKI_KERNEL_SHADER_GENERATED_H
#define MIYUKI_KERNEL_SHADER_GENERATED_H

#include "kerneldef.h"

MYK_KERNEL_NS_BEGIN
struct Shader;
enum ShaderType{
    ESHADER_NONE,
    EFLOAT_SHADER,
    EFLOAT3_SHADER,
    EIMAGE_TEXTURE_SHADER,
    EMIXED_SHADER,
    ESCALED_SHADER,
};

typedef struct FloatShader{
    float value;

}FloatShader;

FloatShader create_float_shader(){
    FloatShader object;
    return object;
}

typedef struct Float3Shader{
    float3 value;
    float multiplier;

}Float3Shader;

Float3Shader create_float3_shader(){
    Float3Shader object;
    return object;
}

typedef struct ImageTextureShader{
    ImageTexture * texture;

}ImageTextureShader;

ImageTextureShader create_image_texture_shader(){
    ImageTextureShader object;
    object.texture= NULL;
    return object;
}

typedef struct MixedShader{
    Shader * fraction;
    Shader * shaderA;
    Shader * shaderB;

}MixedShader;

MixedShader create_mixed_shader(){
    MixedShader object;
    object.fraction= NULL;
    object.shaderA= NULL;
    object.shaderB= NULL;
    return object;
}

typedef struct ScaledShader{
    Shader * scale;
    Shader * shader;

}ScaledShader;

ScaledShader create_scaled_shader(){
    ScaledShader object;
    object.scale= NULL;
    object.shader= NULL;
    return object;
}

typedef struct Shader{
    ShaderType type_tag;
    union{
        FloatShader float_shader;
        Float3Shader float3_shader;
        ImageTextureShader image_texture_shader;
        MixedShader mixed_shader;
        ScaledShader scaled_shader;
    };

}Shader;

#define DISPATCH_SHADER(method,object, ...) \
    switch(type_tag) {\
    case EFLOAT_SHADER:\
        return float_shader##_##method(object, __VA_ARGS__);\
    case EFLOAT3_SHADER:\
        return float3_shader##_##method(object, __VA_ARGS__);\
    case EIMAGE_TEXTURE_SHADER:\
        return image_texture_shader##_##method(object, __VA_ARGS__);\
    case EMIXED_SHADER:\
        return mixed_shader##_##method(object, __VA_ARGS__);\
    case ESCALED_SHADER:\
        return scaled_shader##_##method(object, __VA_ARGS__);\
    };\
    assert(0);\
}
Shader create_shader(ShaderType type_tag){
    switch(type_tag) {
    case EFLOAT_SHADER:
        return create_float_shader();
    case EFLOAT3_SHADER:
        return create_float3_shader();
    case EIMAGE_TEXTURE_SHADER:
        return create_image_texture_shader();
    case EMIXED_SHADER:
        return create_mixed_shader();
    case ESCALED_SHADER:
        return create_scaled_shader();
    };
    assert(0);
}

MYK_KERNEL_NS_END

#endif
