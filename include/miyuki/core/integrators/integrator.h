#ifndef MIYUKI_INTERGRATOR_H
#define MIYUKI_INTERGRATOR_H

#include <reflection.h>
#include <core/film.h>
#include <core/aovrecord.hpp>
#include <core/samplingcontext.hpp>
#include <core/abortable.h>


namespace Miyuki {
	namespace Core {
		class Scene;

		using RenderResultCallback = std::function<void(Arc<Film> film)>;
		using ProgressiveRenderCallback = std::function<void(Arc<Film> film)>;

		struct IntegratorContext {
			Scene* scene = nullptr;
			Camera* camera = nullptr;
			Sampler* sampler = nullptr;
			Arc<Film> film = nullptr;
			RenderResultCallback resultCallback;
		};

		class Integrator : public Reflective, public Abortable {
		public:
			MYK_INTERFACE(Integrator);
		};
		MYK_REFL(Integrator, (Reflective)(Abortable), MYK_REFL_NIL);

		class ProgressiveRenderer : public virtual Integrator {
		public:
			MYK_INTERFACE(ProgressiveRenderer);
			virtual void renderProgressive(
				const IntegratorContext& context,
				ProgressiveRenderCallback progressiveCallback) = 0;
		};
		MYK_REFL(ProgressiveRenderer, (Integrator), MYK_REFL_NIL);
	}
}


#endif