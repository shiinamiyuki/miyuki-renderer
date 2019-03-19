//
// Created by xiaoc on 2019/3/18.
//

#ifndef MIYUKI_MLT_H
#define MIYUKI_MLT_H

#include <integrators/bdpt/bdpt.h>

namespace Miyuki {
    struct MarkovChain {
        Seed seed;
        std::vector<Bidir::Vertex> lightSubPath, lightBackUp;
        std::vector<Bidir::Vertex> cameraSubPath, cameraBackUp;
        Point2i imageLoc;
        Spectrum L;

        void clear() {
            lightSubPath.clear();
            cameraSubPath.clear();
        }

        void backUp() {
            lightBackUp = lightSubPath;
            cameraBackUp = cameraSubPath;
        }

        void restore() {
            cameraSubPath = cameraBackUp;
            lightSubPath = lightBackUp;
        }

        MarkovChain(Seed seed) : seed(seed) {}
    };

    class MLT : public BDPT {
    protected:
        int nBootstrap;
        int nChains;
        int64_t nMutations;
        Float b;
        int nDirect;
        bool progressive;
        std::vector<MarkovChain> chains;

        void generateBootstrapSamples(Scene &scene);

        Spectrum evalMC(Scene &scene, RenderContext &ctx, MarkovChain &markovChain, Point2i *raster);

        Spectrum newPath(Scene &scene, MemoryArena *, MarkovChain &markovChain, Point2i *raster);

    public:
        MLT(const ParameterSet&set);

        void render(Scene &scene) override;
    };
}
#endif //MIYUKI_MLT_H
