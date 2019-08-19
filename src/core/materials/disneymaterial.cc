#include <core/materials/disneymaterial.h>

namespace Miyuki {
	namespace Core {
		static Float DisneyFrWeight(const Float F, Float cosTheta) {
			Float m = (1 - cosTheta);
			Float m2 = m * m;
			return (F - 1) * m2 * m2 * m;
		}
		static Spectrum DisneyDiffuse(const Spectrum& R, const Float FD90, const Float cosThetaI,
			const Float cosThetaO) {
			return R * INVPI
				* (1 + DisneyFrWeight(FD90, cosThetaI)) * (1 + DisneyFrWeight(FD90, cosThetaO));

		}

		static Spectrum DisneySS(const Spectrum& R, const Float Fss90, const Float cosThetaI,
			const Float cosThetaO) {
			auto Fss = (1 + DisneyFrWeight(Fss90, cosThetaI)) * (1 + DisneyFrWeight(Fss90, cosThetaO));
			return 1.25 * R * INVPI * (Fss * (1 / (cosThetaO + cosThetaI) - 0.5) + 0.5);
		}

		BSDFComponent* DisneyMaterial::createBSDF(BSDFCreationContext&)const {
			return nullptr;
		}
	}
}