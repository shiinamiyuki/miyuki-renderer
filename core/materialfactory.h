//
// Created by Shiina Miyuki on 2019/2/6.
//

#ifndef MIYUKI_MATERIALFACTORY_H
#define MIYUKI_MATERIALFACTORY_H

#include "material.h"

namespace Miyuki {
    MaterialInfo makeDiffuse(const Spectrum& R);
    MaterialInfo makeLight(const Spectrum& R);
    class MaterialMaker : public MaterialFactory {
    public:
        MaterialPtr operator()(const MaterialInfo &info) override;
    };
}

#endif //MIYUKI_MATERIALFACTORY_H
