#pragma once

#include <core/medium/medium.h>


namespace Miyuki {
	namespace Core {
		class HomogeneousMedium : public Medium {
			Spectrum sigma_t;
			Spectrum sigma_s;
			Spectrum sigma_a;
		public:
			MYK_CLASS(HomogeneousMedium);		
			Spectrum color_a, color_s;
			Float density_a = 1.0f;
			Float density_s = 1.0f;
			Float g;
			virtual Spectrum Tr(const Ray& ray, Sampler& sampler)const;
			virtual Spectrum sample(MediumSample& sample)const;
			void doPreprocess()override;
		};
		MYK_IMPL(HomogeneousMedium, "Medium.Homogeneous");
		MYK_REFL(HomogeneousMedium, (Medium), (color_s)(color_a)(density_a)(density_s)(g));
	}
}