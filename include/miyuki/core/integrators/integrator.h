#ifndef MIYUKI_INTERGRATOR_H
#define MIYUKI_INTERGRATOR_H

#include <reflection.h>
#include <core/film.h>
#include <core/aovrecord.hpp>
#include <core/samplingcontext.hpp>


namespace Miyuki {
	namespace Core {
		class Scene;

		using RenderResultCallback = std::function<void(Arc<Film> film)>;
		using ProgressiveRenderCallback = std::function<void(Arc<Film> film)>;


		struct Integrator : Component {
			MYK_INTERFACE(Integrator);
		};
		MYK_EXTENDS(Integrator, (Component));

		struct ProgressiveRenderer : virtual Integrator {
			MYK_INTERFACE(ProgressiveRenderer);
			virtual void render(
				Scene& scene,
				Camera& camera,
				ProgressiveRenderCallback progressiveCallback,
				RenderResultCallback resultCallback) = 0;
		};
		MYK_EXTENDS(ProgressiveRenderer, (Integrator));
	}
}


#endif