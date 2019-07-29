#include <core/integrators/samplerintegrator.h>
#include <utils/thread.h>
#include <core/progress.h>
#include <utils/log.h>

namespace Miyuki {
	namespace Core {
		void SamplerIntegrator::renderProgressive(
			const IntegratorContext& context,
			ProgressiveRenderCallback progressiveCallback) {
			_aborted = false;
			auto& scene = *context.scene;
			auto& camera = *context.camera;
			auto& sampler = *context.sampler;
			auto& film = *context.film;
			auto nThreads = Thread::pool->numThreads();
			std::vector<MemoryArena> arenas(nThreads);
			std::vector<Box<Sampler>> samplers(nThreads);
			for (auto& s:samplers) {
				s = std::move(sampler.clone());
			}
			Log::log("Started {0}, total {1} samples, resolution {2}x{3}\n",
				typeInfo()->name(),
				spp,
				film.width(),
				film.height());
			ProgressReporter<size_t> reporter(spp, [&](size_t cur, size_t total) {
				Log::log("Done samples {0}/{1}\n", cur, total);
				progressiveCallback(context.film);
			});

			for (size_t iter = 0; iter < spp && !_aborted; iter++) {
				
				Thread::ParallelFor2D(context.film->imageDimension(), [&](const Point2i& idx, uint32_t threadId) {				
					if (_aborted)return;
					auto ctx = CreateSamplingContext(&camera, &sampler, &arenas[threadId],
						context.film->imageDimension(), idx);
					Li(context, ctx);
					ctx.arena->reset();
				}, TileSize* TileSize);
				reporter.update();
			}
			context.resultCallback(context.film);
		}
	}
}