// AUTO GENERATED. DO NOT EDIT
#ifndef MIYUKI_KERNEL_SHADER_GENERATED_H
#define MIYUKI_KERNEL_SHADER_GENERATED_H

#include "kerneldef.h"

MYK_KERNEL_NS_BEGIN
enum ShaderType{
    SHADER_NONE,
    FLOAT_SHADER,
    FLOAT3_SHADER,
    IMAGE_TEXTURE_SHADER,
    MIXED_SHADER,
    SCALED_SHADER,
    END_SHADER,
};
struct Shader;
struct FloatShader;
struct Float3Shader;
struct ImageTextureShader;
struct MixedShader;
struct ScaledShader;
struct EndShader;

typedef struct Shader{
    ShaderType type_tag;

}Shader;

#define DISPATCH_SHADER(method,object, ...) \
    switch(object->type_tag) {\
    case FLOAT_SHADER:\
        return float_shader##_##method((FloatShader *)object, __VA_ARGS__);\
    case FLOAT3_SHADER:\
        return float3_shader##_##method((Float3Shader *)object, __VA_ARGS__);\
    case IMAGE_TEXTURE_SHADER:\
        return image_texture_shader##_##method((ImageTextureShader *)object, __VA_ARGS__);\
    case MIXED_SHADER:\
        return mixed_shader##_##method((MixedShader *)object, __VA_ARGS__);\
    case SCALED_SHADER:\
        return scaled_shader##_##method((ScaledShader *)object, __VA_ARGS__);\
    case END_SHADER:\
        return end_shader##_##method((EndShader *)object, __VA_ARGS__);\
    };\
    assert(0);

MYK_KERNEL_FUNC_INLINE void create_shader(Shader* object){
}

typedef struct FloatShader{
    Shader _base;
    float value;

}FloatShader;

MYK_KERNEL_FUNC_INLINE void create_float_shader(FloatShader* object){
    create_shader((Shader *)object);
    object->_base.type_tag = FLOAT_SHADER;
}

typedef struct Float3Shader{
    Shader _base;
    float3 value;
    float multiplier;

}Float3Shader;

MYK_KERNEL_FUNC_INLINE void create_float3_shader(Float3Shader* object){
    create_shader((Shader *)object);
    object->_base.type_tag = FLOAT3_SHADER;
}

typedef struct ImageTextureShader{
    Shader _base;
    ImageTexture * texture;

}ImageTextureShader;

MYK_KERNEL_FUNC_INLINE void create_image_texture_shader(ImageTextureShader* object){
    object->texture = NULL;
    create_shader((Shader *)object);
    object->_base.type_tag = IMAGE_TEXTURE_SHADER;
}
/*eval B, A, fraction*/
typedef struct MixedShader{
    Shader _base;

}MixedShader;

MYK_KERNEL_FUNC_INLINE void create_mixed_shader(MixedShader* object){
    create_shader((Shader *)object);
    object->_base.type_tag = MIXED_SHADER;
}
/*eval v, k*/
typedef struct ScaledShader{
    Shader _base;

}ScaledShader;

MYK_KERNEL_FUNC_INLINE void create_scaled_shader(ScaledShader* object){
    create_shader((Shader *)object);
    object->_base.type_tag = SCALED_SHADER;
}

typedef struct EndShader{
    Shader _base;

}EndShader;

MYK_KERNEL_FUNC_INLINE void create_end_shader(EndShader* object){
    create_shader((Shader *)object);
    object->_base.type_tag = END_SHADER;
}

MYK_KERNEL_NS_END

#endif
