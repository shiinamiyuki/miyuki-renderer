//
// Created by Shiina Miyuki on 2019/2/6.
//

#ifndef MIYUKI_MATERIALFACTORY_H
#define MIYUKI_MATERIALFACTORY_H

#include "material.h"

namespace Miyuki {
    enum BSDFNodeType{
        matte,
        glass,
        glossyGGX,
    };
    class MaterialMaker : public MaterialFactory {
    public:
        MaterialPtr operator()(const MaterialInfo &info) override;
    };
}

#endif //MIYUKI_MATERIALFACTORY_H
