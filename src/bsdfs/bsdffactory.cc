//
// Created by Shiina Miyuki on 2019/2/9.
//

#include "bsdffactory.h"
#include "lambertian.h"
#include "oren-nayar.h"
#include "specular.h"
#include "fresnel.h"
#include "microfacet.h"

using namespace Miyuki;

struct MyString : public std::string {
public:
    MyString(const std::string &s) : std::string(s) {}

    bool operator==(const char *s) const {
        auto dist = editDistance(*this, s, false);
        if (length() <= 5)
            return dist == 0;
        else
            return dist <= 3;
    }
};

std::shared_ptr<BSDF> BSDFFactory::operator()(const MaterialInfo &materialInfo) {
    std::shared_ptr<BSDF> bsdf;
    MyString bsdfType = materialInfo.bsdfType;
    const auto &kd = materialInfo.kd;
    const auto &ks = materialInfo.ks;
    try {
        if (bsdfType == "default") {
            bsdf = std::make_shared<LambertianBSDF>(kd);
        } else if (bsdfType == "lambertian") {
            bsdf = std::make_shared<LambertianBSDF>(kd);
        } else if (bsdfType == "OrenNayar") {
            bsdf = std::make_shared<OrenNayarBSDF>(std::stof(materialInfo.parameters.at("sigma")), kd);
        } else if (bsdfType == "microfacet") {
            
        } else {
            fmt::print(stderr, "Unrecognized bsdf type: {}\n", bsdfType);
            bsdf = std::make_shared<LambertianBSDF>(kd);
        }
    } catch (std::exception &e) {
        fmt::print(stderr, "Exception in BSDFFactory: {}\n", e.what());
        bsdf = std::make_shared<LambertianBSDF>(kd);
    }
    bsdf->ka = materialInfo.ka;
    return bsdf;
}
