//
// Created by Shiina Miyuki on 2019/3/11.
//

#ifndef MIYUKI_PATHMLT_H
#define MIYUKI_PATHMLT_H

#include <integrators/integrator.h>
#include <integrators/volpath/volpath.h>
#include <bidir/vertex.h>
#include <bidir/mutation.h>
#include <integrators/metropolis.h>
#include <core/scene.h>


namespace Miyuki {
    // Unidirectional Path Space MLT
    class PathMLT : public VolPath {
    public:
        struct MChain {
            Bidir::SubPath path, pathBackup;
            std::vector<Bidir::Vertex> data, dataBackup;

            void restore() {
                path = pathBackup;
                data = dataBackup;
            }

            void backup() {
                pathBackup = path;
                dataBackup = data;
            }
        };

    protected:
        Float largeStepProbability;
        int luminanceSamples;
        int directSamples;
        int nChains;

        std::vector<MChain> paths;

        class Bootstrapper : public MetropolisBootstrapper {
        public:
            Scene &scene;
            PathMLT *integrator;

            Bootstrapper(Scene &scene, PathMLT *integrator) : scene(scene), integrator(integrator) {}

            Float f(Seed *seed, MemoryArena *) override;
        };

        friend class Bootstrapper;

        Float mutate(MChain &);// returns accept prob
    public:
        PathMLT(const ParameterSet &set);

        void render(Scene &scene) override;

    protected:
    };
}
#endif //MIYUKI_PATHMLT_H
