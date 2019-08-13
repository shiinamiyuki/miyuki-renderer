#include <core/integrators/samplerintegrator.h>
#include <utils/thread.h>
#include <core/progress.h>
#include <utils/log.h>
#include <hilbert/hilbert_curve.hpp>
#include <core/scene.h>

namespace Miyuki {
	namespace Core {
		void HilbertMapping(const Point2i& nTiles, std::vector<Point2f>& hilbertMapping) {
			int M = std::ceil(std::log2(std::max(nTiles.x, nTiles.y)));

			for (size_t i = 0; i < pow(2, M + M); i++) {
				int tx, ty;
				::d2xy(M, i, tx, ty);
				if (tx >= nTiles.x || ty >= nTiles.y)
					continue;
				hilbertMapping.emplace_back(tx, ty);
			}
			size_t mid = (hilbertMapping.size() / 2);
			for (size_t i = 0; i < mid / 2; i++) {
				std::swap(hilbertMapping[i], hilbertMapping[mid - i - 1ULL]);
			}
		}
		constexpr size_t rayN = 8;
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
			std::vector<Box<Sampler>> samplers;
			samplers.reserve(film.width() * film.height());
			for (auto j = 0; j < film.height(); j++) {
				for (auto i = 0; i < film.width(); i++) {

					auto s = sampler.clone();
					SamplerState state(film.imageDimension(), Point2i(i, j), 0U, spp);
					s->start(state);
					samplers.emplace_back(std::move(s));
				}
			}

			Point2i nTiles = film.imageDimension() / TileSize + Point2i{ 1, 1 };
			std::vector<Point2f> hilbertMapping;
			HilbertMapping(nTiles, hilbertMapping);
			Log::log("Started {0}, total {1} samples, resolution {2}x{3}\n",
				typeInfo()->name(),
				spp,
				film.width(),
				film.height());
			double elapsedRenderTime = 0;
			std::unique_ptr<ProgressReporter<size_t>> reporter;
			reporter.reset(new ProgressReporter<size_t>(spp, [&](size_t cur, size_t total) {
				Log::log("Done samples {0}/{1}, traced {2} rays, {3}M rays/sec\n",
					cur,
					total,
					scene.getRayCount(),
					scene.getRayCount() / elapsedRenderTime /1e6);
				progressiveCallback(context.film);
			}));
			scene.resetRayCount();
			renderStart(context);
			for (size_t iter = 0; iter < spp && !_aborted; iter++) {
				double start = reporter->elapsedSeconds();
				Thread::ParallelFor(0u, hilbertMapping.size(), [&](uint32_t idx, uint32_t threadId) {
					int tx, ty;
					tx = hilbertMapping[idx].x;
					ty = hilbertMapping[idx].y;
					Point2i tilePos(tx, ty);
					tilePos *= TileSize;
					Bound2i tileBound(tilePos, tilePos + Point2i{ TileSize, TileSize });
					if constexpr (rayN == 1) {
						for (int i = 0; i < TileSize; i++) {
							for (int j = 0; j < TileSize; j++) {
								int x = tx * TileSize + i;
								int y = ty * TileSize + j;
								if (x >= film.width() || y >= film.height())
									continue;
								auto raster = Point2i{ x, y };
								SamplingContext ctx = CreateSamplingContext(&camera,
									samplers.at(x + y * film.width()).get(), &arenas[threadId], film.imageDimension(), raster);
								Intersection isct;
								scene.intersect(ctx.primary, &isct);
								Li(&isct, context, ctx);
							}
						}
					}
					else if (rayN == 8) {
						for (int i = 0; i < TileSize; i += 2) {
							for (int j = 0; j < TileSize; j += 4) {
								Ray8 ray8;
								Intersection8 isct8;
								SamplingContext ctx[8];
								for (int k = 0; k < 2; k++) {
									for (int m = 0; m < 4; m++) {
										int x = tx * TileSize + i + k;
										int y = ty * TileSize + j + m;
										if (x >= film.width() || y >= film.height())
											continue;
										auto raster = Point2i{ x, y };
										ctx[4 *k + m] = CreateSamplingContext(&camera,
											samplers.at(x + y * film.width()).get(), &arenas[threadId], film.imageDimension(), raster);
										ray8.rays[4 * k + m] = ctx[4 * k + m].primary;
									}
								}
								scene.intersect8(ray8, &isct8);
								for (int idx = 0; idx < 8; idx++) {
									if(ray8.rays[idx].valid())
										Li(&isct8.isct[idx], context, ctx[idx]);
								}
							}
						}
					}
					arenas[threadId].reset();
				}, 64);
				elapsedRenderTime += reporter->elapsedSeconds() - start;
				reporter->update();
			}
			renderEnd(context);
			context.resultCallback(context.film);
		}
	}
}