#pragma once

#include <core/denoiser/denoiser.h>
#include <OpenImageDenoise/oidn.hpp>

namespace Miyuki {
	namespace Core {
		class OIDNDenoiserImpl;
		class OIDNDenoiser: public Denoiser {
			std::shared_ptr<OIDNDenoiserImpl> impl;
		public:
			MYK_CLASS(OIDNDenoiser);
			OIDNDenoiser();
			virtual void setup(const Point2i& dimension);
			virtual void addSample(const Point2f& pixel, const AOVRecord& sample, Float weight = 1.0f);
			virtual bool denoise(IO::Image&);
		};
		MYK_IMPL(OIDNDenoiser, "Denoiser.OIDN");
		MYK_REFL(OIDNDenoiser, (Denoiser), MYK_REFL_NIL);
	}
}