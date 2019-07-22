#include <core/integrators/ao.h>
#include <utils/thread.h>

namespace Miyuki {
	namespace Core {
		void AOIntegrator::renderProgressive(
			const IntegratorContext& context,
			ProgressiveRenderCallback progressiveCallback) {
			_aborted = false;
			auto& scene = *context.scene;
			auto& camera = *context.camera;
			auto& sampler = *context.sampler;
			auto nThreads = Thread::pool->numThreads();
			std::vector<MemoryArena> arenas(nThreads);
			for (size_t iter = 0; iter < spp; iter++) {
				Thread::ParallelFor2D(context.film->imageDimension(), [&](const Point2i& idx, uint32_t threadId) {

				});
			}
			context.resultCallback(context.film);
		}
		void AOIntegrator::abort() {
			_aborted = true;
		}
	}
}