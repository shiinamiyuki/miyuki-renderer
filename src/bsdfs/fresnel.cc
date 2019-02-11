//
// Created by Shiina Miyuki on 2019/2/10.
//

#include "fresnel.h"

using namespace Miyuki;

Spectrum FresnelDielectric::eval(Float cosI) const {
    auto x = FrDielectric(cosI, etaI, etaT);
    return {x, x, x};
}
