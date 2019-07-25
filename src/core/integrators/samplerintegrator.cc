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
			Log::log("Started {0}, total {1} samples\n", typeInfo()->name(), spp);
			ProgressReporter<size_t> reporter(spp, [&](size_t cur, size_t total) {
				Log::log("Done sample {0}/{1}\n", cur, total);
				progressiveCallback(context.film);
			});

			for (size_t iter = 0; iter < spp && !_aborted; iter++) {
				Thread::ParallelFor2D(context.film->imageDimension(), [&](const Point2i& idx, uint32_t threadId) {
					if (_aborted)return;
					for (int x = 0; x < TileSize; x++) {
						for (int y = 0; y < TileSize; y++) {
							if (_aborted)return;
							Point2i raster = idx + Point2i{ x, y };
							auto ctx = CreateSamplingContext(&camera, &sampler, &arenas[threadId], context.film->imageDimension(), raster);
							Li(context, ctx);
						}
					}
				}, TileSize);
				reporter.update();
			}
			context.resultCallback(context.film);
		}
	}
}