//
// Created by Shiina Miyuki on 2019/2/9.
//

#include "bsdffactory.h"
using namespace Miyuki;

std::shared_ptr<BSDF> BSDFFactory::operator()(const MaterialInfo &materialInfo) {
    auto bsdf = std::make_shared<LambertianBSDF>(materialInfo.kd);
    if(materialInfo.ka.maxReflectance > 0.01){
        bsdf->ka = materialInfo.ka;
    }
    return bsdf;
}
