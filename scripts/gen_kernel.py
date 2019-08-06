from class_generator import *
kernel_shader = \
"""// AUTO GENERATED. DO NOT EDIT
#ifndef MIYUKI_KERNEL_SHADER_GENERATED_H
#define MIYUKI_KERNEL_SHADER_GENERATED_H

#include "kerneldef.h"

MYK_KERNEL_NS_BEGIN
"""


def create_class(name, data:dict):
    _super = data['super']
    _class = KernelClass(name,_super)
    if 'attr' in data:
        attrs = data['attr']
        for attr in attrs:
            ty = attrs[attr]
            _class.add_attr(KernelAttribute(attr, ty))    
    return _class
def create_classes(data):
    for i in data:
        create_class(i, data[i])
shaders = {
    'Shader':{
        'super':''
    },
    'FloatShader':{
        'super':'Shader',
        'attr':{
            'value':'float'
        }
    },
    'Float3Shader':{
        'super':'Shader',
        'attr':{
            'value':'float3',
            'multiplier':'float'
        }
    },
    'ImageTextureShader':{
        'super':'Shader',
        'attr':{
            'texture':'ImageTexture *',
        }
    },
    'MixedShader':{
        'super':'Shader',
        'attr':{
            'fraction':'Shader *',
            'shaderA':'Shader *',
            'shaderB':'Shader *'
        }
    },
    'ScaledShader':{
        'super':'Shader',
        'attr':{
            'scale':'Shader *',
            'shader':'Shader *',
        }
    }
}
create_classes(shaders)
kernel_shader += globals.gen()

kernel_shader +=\
"""
MYK_KERNEL_NS_END

#endif
"""

open("../include/miyuki/kernel/shader.generated.h", "w").write(kernel_shader)