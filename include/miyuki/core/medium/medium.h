#pragma once

#include <reflection.h>

namespace Miyuki {
	namespace Core {
		class Medium : public Reflective {
		public:
			MYK_INTERFACE(Medium);
		};
		MYK_REFL(Medium, (Reflective), MYK_REFL_NIL);
	}
}