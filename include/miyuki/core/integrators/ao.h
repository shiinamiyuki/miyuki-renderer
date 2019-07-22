#ifndef MIYUKI_AO_H
#define MIYUKI_AO_H

#include <core/integrators/integrator.h>

namespace Miyuki {
	namespace Core {
		class AOIntegrator final : public ProgressiveRenderer {
		public:
			size_t spp = 16;
			Float occlusionDistance = 1e5;
			MYK_CLASS(AOIntegrator);
			void renderProgressive(
				const IntegratorContext& context,
				ProgressiveRenderCallback progressiveCallback)override;
			void abort()override;
			bool aborted()const override { return _aborted; }
			void restart()override { _aborted = true; }
			AOIntegrator() :_aborted(false) {}
		private:
			std::atomic<bool> _aborted;
		};
		MYK_REFL(AOIntegrator, (spp)(occlusionDistance));
		MYK_IMPL(AOIntegrator, (ProgressiveRenderer), "Integrator.AO");
	}
}

#endif