#pragma once
#include "Miyuki.h"
#include "vec.h"
#include "Ray.h"
namespace Miyuki {
	class Scene;
	class Integrator {
	public:
		Integrator() {}
		virtual void render(Scene * scene) = 0;
		//virtual vec3 finalRender(Scene *scene) = 0;
	};
	

}