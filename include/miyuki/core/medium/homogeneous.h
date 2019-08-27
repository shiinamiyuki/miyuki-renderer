#pragma once

#include <core/medium/medium.h>


namespace Miyuki {
	namespace Core {
		class HomogeneousMedium : public Medium {
			Spectrum sigma_t;
		public:
			MYK_CLASS(HomogeneousMedium);
			Spectrum sigma_s;
			Spectrum sigma_a;
			Float g;
			virtual Spectrum Tr(const Ray& ray, Sampler& sampler)const;
			virtual Spectrum sample(MediumSample& sample)const;
			void doPreprocess()override;
		};
		MYK_IMPL(HomogeneousMedium, "Medium.Homogeneous");
		MYK_REFL(HomogeneousMedium, (Medium), (sigma_a)(sigma_s)(g));
	}
}