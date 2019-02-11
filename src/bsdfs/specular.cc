//
// Created by Shiina Miyuki on 2019/2/10.
//

#include "specular.h"
using namespace Miyuki;

Spectrum SpecularTransmissionBSDF::sample(ScatteringEvent &event) const {
    bool entering = cosTheta(event.wo) > 0;
    Float etaI = entering ? etaA : etaB;
    Float etaT = entering ? etaB : etaA;
    auto n = Vec3f{0,1,0};
    if(Vec3f::dot(n,event.wo) < 0){
        n *= -1;
    }
    Vec3f wi;
    if(!refract(event.wo,n,etaI/etaT,&wi)){
        return {};
    }
    event.setWi(wi);
    event.pdf  = 1;
    Spectrum ft = Spectrum(evalAlbedo(event) * (Spectrum(1.0f,1.0f,1.0f) - fresnel.eval(cosTheta(event.wi))));
    ft /= absCosTheta(event.wi);
    return ft;
}
