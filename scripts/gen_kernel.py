from class_generator import *
kernel_shader = \
"""// AUTO GENERATED. DO NOT EDIT
#ifndef MIYUKI_KERNEL_SHADER_GENERATED_H
#define MIYUKI_KERNEL_SHADER_GENERATED_H

#include "kerneldef.h"

MYK_KERNEL_NS_BEGIN
"""

shaders = {
    'Shader':{
        'super':'',
        
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
            'texture':'ImageTexture',
        }
    },
    'MixedShader':{
        'super':'Shader',
        'attr':{
        },
        'comment':
            "eval B, A, fraction"
    },
    'ScaledShader':{
        'super':'Shader',
        'attr':{
            
        },'comment':
            "eval v, k"
    },
    'EndShader':{
        'super':'Shader'
    }
}
create_classes(shaders)
kernel_shader += globals.gen()

kernel_shader +=\
"""
MYK_KERNEL_NS_END

#endif
"""

open("../include/miyuki/kernel/generated.shader.h", "w").write(kernel_shader)

globals.clear()

kernel_material =  \
"""// AUTO GENERATED. DO NOT EDIT
#ifndef MIYUKI_KERNEL_MATERIAL_GENERATED_H
#define MIYUKI_KERNEL_MATERIAL_GENERATED_H

#include "kerneldef.h"
#include "kernel_shaderdata.h"
#include "kernel_bsdf.h"

MYK_KERNEL_NS_BEGIN
"""
materials = {
    "Material":{
        'super':None,
        'attr':{
            'lobe':'BSDFLobe'
        }
    },
    'DiffuseMaterial':{
        'super':'Material',
        'attr':{
            'roughness':'ShaderData',
            'color':'ShaderData'
        }
    }, 
    'GlossyMaterial':{
        'super':'Material',
        'attr':{
            'roughness':'ShaderData',
            'color':'ShaderData'
        }
    },
     'MixedMaterial':{
        'super':'Material',
        'attr':{
            'fraction':'ShaderData',
            'matA':'int',
            'matB':'int'
        }
    },
    'NullMaterial':{
        'super':'Material',
        'attr':{}
    }
}
create_classes(materials)
kernel_material += globals.gen()
kernel_material +=\
"""
MYK_KERNEL_NS_END

#endif
"""
open("../include/miyuki/kernel/generated.material.h", "w").write(kernel_material)
globals.clear()
 
kernel_light = \
"""// AUTO GENERATED. DO NOT EDIT
#ifndef MIYUKI_KERNEL_LIGHT_GENERATED_H
#define MIYUKI_KERNEL_LIGHTL_GENERATED_H

#include "kerneldef.h"
#include "kernel_shader.h"
MYK_KERNEL_NS_BEGIN
"""

lights = {
    "Light":{
        'super':None,
        'attr':{
            
        }
    },
    "AreaLight":{
        'super':'Light',
        'attr':{
            'primitive':'Primitive * '
        }
    },
    "InfiniteAreaLight":{
        'super':'Light',
        'attr':{
            'shader':'Shader *',
            'distribution':'Distribution2D'
        }
    }

}
create_classes(lights)
kernel_light += globals.gen()

kernel_light +=\
"""
MYK_KERNEL_NS_END

#endif
"""
open("../include/miyuki/kernel/generated.light.h", "w").write(kernel_light)