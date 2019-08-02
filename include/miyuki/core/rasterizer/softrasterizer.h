#include <core/rasterizer/rasterizer.h>

namespace Miyuki {
	namespace Core {
		class SoftRasterizer : public Rasterizer {
		public:
			MYK_CLASS(SoftRasterizer);
			virtual void render(RasterizationContext& ctx)override; 
		};
		MYK_IMPL(SoftRasterizer, "Rasterizer.Soft");
		MYK_REFL(SoftRasterizer, (Rasterizer), MYK_REFL_NIL);
	}
}