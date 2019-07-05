#ifndef MIYUKI_CAMERA_H
#define MIYUKI_CAMERA_H

#include <reflection.h>


namespace Miyuki {
	namespace Core {
		struct Camera : Trait {

		};

		struct PerspectiveCamera final: Trait {

		};
	}
}

MYK_REFL(Miyuki::Core::PerspectiveCamera);
#endif