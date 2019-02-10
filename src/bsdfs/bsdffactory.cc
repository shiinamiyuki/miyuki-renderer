//
// Created by Shiina Miyuki on 2019/2/9.
//

#include "bsdffactory.h"
#include "lambertian.h"
#include "oren-nayar.h"

using namespace Miyuki;

std::shared_ptr<BSDF> BSDFFactory::operator()(const MaterialInfo &materialInfo) {
    auto bsdf = std::make_shared<OrenNayarBSDF>(0.3, materialInfo.kd);
    bsdf->ka = materialInfo.ka;
    return bsdf;
}
