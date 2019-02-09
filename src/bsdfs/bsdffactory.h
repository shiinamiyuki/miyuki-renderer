//
// Created by Shiina Miyuki on 2019/2/9.
//

#ifndef MIYUKI_BSDFFACTORY_HPP
#define MIYUKI_BSDFFACTORY_HPP

#include "bsdf.h"

namespace Miyuki {
    class BSDF;

    struct MaterialInfo;

    class BSDFFactory {
    public:
        virtual std::shared_ptr<BSDF> operator()(const MaterialInfo &);
    };
}
#endif //MIYUKI_BSDFFACTORY_HPP
