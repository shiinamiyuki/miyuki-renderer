#pragma once
#include <miyuki.h>
#include <io/image.h>
#include <core/aovrecord.hpp>
#include <bcd/bcd/SamplesAccumulator.h>
#include <reflection.h>
#include <core/denoiser/denoiser.h>

namespace Miyuki {
	namespace Core {
		struct DenoiserHistogramParameters :bcd::HistogramParameters {

		};
		class AOVDenoiser {
			Point2i dimension;
			bcd::SamplesAccumulator accumulator;
		public:
			AOVDenoiser(const Point2i& dimension, const DenoiserHistogramParameters&);
			void addSample(const Point2f& pixel, const Spectrum& color, Float weight = 1.0f);
			bool denoise(IO::Image&);


		};
		
		class DenoiserDriver {
			Point2i dimension;
			std::unique_ptr<AOVDenoiser> aov[AOVCount];
		public:
			DenoiserDriver(const Point2i& dimension, const DenoiserHistogramParameters&);
			void addSample(AOVType type, const Point2f& pixel, const Spectrum& color, Float weight = 1.0f) {
				aov[type]->addSample(pixel, color, weight);
			}
			bool denoise(IO::Image&);
		};

		class BCDDenoiser : public Denoiser {
			std::unique_ptr<DenoiserDriver> driver;
		public:
			MYK_CLASS(BCDDenoiser);
			BCDDenoiser() = default;
			virtual void setup(const Point2i& dimension)override;
			virtual void addSample(const Point2f& pixel, const AOVRecord& sample, Float weight);
			virtual bool denoise(IO::Image&);
		};
		MYK_IMPL(BCDDenoiser, "Denoiser.BCD");
		MYK_REFL(BCDDenoiser, (Denoiser), MYK_REFL_NIL);
	}
}