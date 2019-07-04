#ifndef MIYUKI_BSDF_MICROFACET_HPP
#define MIYUKI_BSDF_MICROFACET_HPP

#include <core/probablity/commonpdf.hpp>

namespace Miyuki {
	namespace Core {
		template<class Model>
		struct MicrofacetModel {
			Float D(const Vec3f& wh) const {
				return This().DImpl(Wh);
			}
			Float lambda(const Vec3f& w) const {
				return This().lambdaImpl(w);
			}
			Float G1(const Vec3f& w) const {
				return 1 / (1 + lambda(w));
			}
			Float G(const Vec3f& wo, const Vec3f& wi) const {
				return 1 / (1 + lambda(wo) + lambda(wi));
			}
			virtual Vec3f sampleWh(const Vec3f& wo,
				const Point2f& u) const {
				return This().sampleWhImpl(wo, u);
			}
			Float evaluatePdf(const Vec3f& wo, const Vec3f& wh) const {

			}
		private:
			const Model& This()const {
				return *static_cast<Model*>(this);
			}
		};
	}
}


#endif