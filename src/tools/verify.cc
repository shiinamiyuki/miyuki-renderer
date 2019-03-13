//
// Created by xiaoc on 2019/3/12.
//

#include <bsdfs/bsdf.h>
#include <core/scatteringevent.h>
#include <samplers/sobol.h>
#include <bsdfs/lambertian.h>
#include <math/sampling.h>
#include <bsdfs/microfacet.h>

using namespace Miyuki;

void Test(BxDF *bxdf) {
    constexpr int N = 1000000;
    double sum = 0, sumL = 0;
    Seed seed(2934825);
    ScatteringEvent event;
    SobolSampler sampler(&seed);
    sampler.start();
    event.wo = Vec3f{0, 1, 0};//CosineWeightedHemisphereSampling(sampler.get2D()).normalized();
    int n = 0;
    // fmt::print("{}\n",acos(AbsCosTheta(event.wo)));
    // fmt::print("{}\n", bxdf->pdf(event));return;
    Float m = 1e64;
    for (int i = 0; i < N; i++) {
        sampler.start();
        event.u = sampler.get2D();

        auto f = bxdf->sample(event);
        auto pdf = bxdf->pdf(event);
        if (pdf != 0)
            m = std::min(m, pdf);
        Assert(event.pdf >= 0);
        Assert(AbsCosTheta(event.wi) <= 1);
        //fmt::print("{} {}\n", acos(AbsCosTheta(event.wi)),event.pdf);
        if (event.pdf > 0 && !f.isBlack()) {
            if (std::abs(pdf - event.pdf) / pdf > 0.01) {
                fmt::print("{} {}\n", pdf, event.pdf);
                //   std::exit(-1);
            }
            sumL += f[0] * AbsCosTheta(event.wi) / event.pdf;
            sum += 1 / event.pdf;
            n++;
        }
    }
    double I = sum / n;
    fmt::print("I(pdf) = {}, L = {},  error = {}, {}\n", I, sumL / n, 2 * PI - I, m);
}
void Test3(BxDF *bxdf) {
    constexpr int N = 1000000;
    double sum = 0;
    Seed seed(2934825);
    ScatteringEvent event;
    SobolSampler sampler(&seed);
    sampler.start();
    event.wo = CosineWeightedHemisphereSampling(sampler.get2D()).normalized();
    for (int i = 0; i < N; i++) {
        sampler.start();
        event.u = sampler.get2D();
        event.wi = CosineWeightedHemisphereSampling(event.u);
        auto pdf = bxdf->pdf(event);
        Assert(AbsCosTheta(event.wi) <= 1);
        if (pdf > 0) {
            sum += pdf * PI / CosTheta(event.wi);
        }
    }
    double I = sum / N;
    fmt::print("I(pdf) = {},  error = {}\n", I, 1 - I);
}
void plot(const std::string &filename, BxDF *bxdf) {
    constexpr int N = 1000;
    double sum = 0, sumL = 0;
    Seed seed(2934825);
    ScatteringEvent event;
    SobolSampler sampler(&seed);
    sampler.start();
    event.wo = CosineWeightedHemisphereSampling(sampler.get2D()).normalized();
    int n = 0;
    std::ofstream out(filename);
    for (int i = 0; i < N; i++) {
        sampler.start();
        event.u = sampler.get2D();
        event.wi = CosineWeightedHemisphereSampling(event.u);
        auto f = bxdf->f(event);
        //auto pdf = bxdf->pdf(event);
        out << fmt::format("{} {} {}\n", event.wi.x() * f[0], event.wi.z() * f[0], event.wi.y() * f[0]);
    }
}

void Test2(Float r) {
    const int N = 1000000;
    double sum = 0;
    Seed seed(rand());
    SobolSampler sampler(&seed);
    MicrofacetDistribution distribution(MicrofacetModel::beckmann, r, r);
    for (int i = 0; i < N; i++) {
        sampler.start();
        auto wh = CosineWeightedHemisphereSampling(
                sampler.get2D());//distribution.sampleWh(Vec3f{0, 1, 0}, sampler.get2D());
        auto pdf = distribution.pdf(Vec3f{0, 1, 0}, wh);
        if (pdf != 0)
            sum += pdf / (AbsCosTheta(wh) * INVPI);
    }
    double I = sum / N;
    fmt::print("I(pdf) = {},  error = {}\n", I, 1 - I);
}

int main() {
    Test(new LambertianReflection(Spectrum(1, 1, 1)));
    for (double r = 0.01; r < 1; r += 0.05) {
        Test3(new MicrofacetReflection({1, 1, 1},
                                      MicrofacetDistribution(MicrofacetModel::beckmann, r, r),
                                      new PerfectSpecularFresnel()));
    //    Test2(r);
    }
//    Float r = 0.9;
//    plot("../src/scripts/data.txt", new MicrofacetReflection({1, 1, 1},
//                                      MicrofacetDistribution(MicrofacetModel::beckmann, r,r),
//                                      new PerfectSpecularFresnel()));
//    plot("../src/scripts/data.txt", new LambertianReflection({1,1,1}));
//    system("py ../src/scripts/testplot.py");
}