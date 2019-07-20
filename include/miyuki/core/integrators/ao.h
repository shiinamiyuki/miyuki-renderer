#ifndef MIYUKI_AO_H
#define MIYUKI_AO_H

#include <core/integrators/integrator.h>

namespace Miyuki {
	namespace Core {
		struct AOIntegrator final : Integrator, ProgressiveRenderer{
			size_t spp = 16;
			Float occlusionDistance = 1e5;
			MYK_META(AOIntegrator);
			void render(
				Scene& scene,
				Camera& camera,
				ProgressiveRenderCallback progressiveCallback,
				RenderResultCallback resultCallback)override;

		};
		MYK_REFL(AOIntegrator, (spp)(occlusionDistance));
		MYK_IMPL(AOIntegrator, Integrator, "Integrator.AO");
	}
}



#endif