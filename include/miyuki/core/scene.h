#ifndef MIYUKI_SCENE_H
#define MIYUKI_SCENE_H
#include <reflection.h>
#include <materials/material.h>

namespace Miyuki {
	class Materials : Reflection::Object {
	public:
		MYK_CLASS(Materials, Reflection::Object);
		MYK_BEGIN_PROPERTY;
		MYK_PROPERTY(Reflection::Array<Material>, materials);
		MYK_END_PROPERTY;
	};
	class Scene : Reflection::Object {
	public:
		MYK_CLASS(Scene, Reflection::Object);
	};
}


#endif