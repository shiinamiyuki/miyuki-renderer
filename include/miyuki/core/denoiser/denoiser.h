#pragma once

#include <reflection.h>
#include <core/aovrecord.hpp>
#include <io/image.h>

namespace Miyuki {
	namespace Core {
		class Denoiser : public Reflective {
		public:
			MYK_INTERFACE(Denoiser);
			virtual void setup(const Point2i& dimension) = 0;
			virtual void addSample(const Point2f& pixel, const AOVRecord& sample, Float weight=1.0f) = 0;
			virtual bool denoise(IO::Image&) = 0;
		};
		MYK_REFL(Denoiser, (Reflective), MYK_REFL_NIL);
	}
}