#pragma once
#include "Miyuki.h"
namespace Miyuki {
	struct Camera
	{
		vec3 pos;
		vec3 dir;
		Float aov;
		Camera();
		~Camera();
	};
}

