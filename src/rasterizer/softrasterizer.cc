#include <core/rasterizer/softrasterizer.h>
#include <core/scene.h>

namespace Miyuki {
	namespace Core {
		
		Arc<RasterBuffer> SoftRasterizer::initBuffer(size_t w, size_t h, size_t MSAA) {
			/*auto k = std::sqrt(MSAA);
			w *= k;
			h *= k;*/
			return makeArc<RasterBuffer>(w, h);
		}

		void SoftRasterizer::render(RasterizationContext& ctx) {
			auto& scene = *ctx.scene;
			auto buffer = initBuffer(ctx.film->width(), ctx.film->height(), 1);
			for (const auto& mesh : scene.getInstances()) {
				auto& vertices = mesh->getVerticies();
			}
		}
	}
}