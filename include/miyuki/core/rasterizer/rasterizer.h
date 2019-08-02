#include <reflection.h>
#include <core/cameras/camera.h>
namespace Miyuki {
	namespace Core {
		class Scene;
		class PerspectiveCamera;
		struct RasterizationContext {
			Scene* scene = nullptr;
			Box<PerspectiveCamera> camera = nullptr;
			Arc<Film> film = nullptr;
		};
		class Rasterizer : public Reflective {
		public:
			MYK_INTERFACE(Rasterizer);
			virtual void render(RasterizationContext& ctx) = 0;
		};
		MYK_REFL(Rasterizer, (Reflective), MYK_REFL_NIL);
	}
}