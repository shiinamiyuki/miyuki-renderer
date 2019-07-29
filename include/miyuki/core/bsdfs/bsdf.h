#pragma once

#include <miyuki.h>
#include <core/bsdfs/scatteringfunction.hpp>
#include <math/transform.h>
#include <core/samplers/sampler.h>
#include <core/intersection.hpp> 

namespace Miyuki {
	namespace Core {
		enum BSDFLobe {
			ENone = 0,
			EDiffuse = 1 << 0,
			EGlossy = 1 << 2,
			ESpecular = 1 << 3,
			EReflection = 1 << 4,
			ETransmission = 1 << 5,
			EAll = EDiffuse | ESpecular | EGlossy | EReflection | ETransmission,
			EAllButSpecular = EAll & ~ESpecular
		};

		// Some user specific sample option
		enum BSDFSampleOption {
			ENoSampleOption = 0,
			EUseStrictNormal = 1,
		};

		class Sampler;

		struct BSDFSample : ScatteringFunctionSample {
			Float uPick; // picking bsdf
			BSDFLobe lobe; //sampled lobe
			BSDFSampleOption option;

			BSDFSample(Intersection& isct, Sampler* sampler, BSDFSampleOption opt)
				:option(opt), uPick(sampler->get1D()) {
				wo = isct.worldToLocal(isct.wo);
				u = sampler->get2D();
			}
		};
		class BSDFImpl {
			BSDFLobe lobe;
		public:
			BSDFImpl(BSDFLobe lobe) :lobe(lobe) {}
			BSDFLobe getLobe()const { return lobe; }
			bool match(BSDFLobe lobe)const {
				return this->lobe & lobe;
			}
			bool isDelta()const {
				return match(ESpecular);
			}
			virtual void sample(
				BSDFSample& sample
			)const = 0;

			// evaluate bsdf according to wo, wi
			virtual Spectrum evaluate(
				const Vec3f& wo,
				const Vec3f& wi,
				BSDFSampleOption option,
				BSDFLobe lobe = BSDFLobe::EAll
			)const = 0;

			// evaluate pdf according to wo, wi
			virtual Float evaluatePdf(
				const Vec3f& wo,
				const Vec3f& wi,
				BSDFSampleOption option,
				BSDFLobe lobe = BSDFLobe::EAll
			)const = 0;

		};
		class BSDF {
			CoordinateSystem localFrame;
			Vec3f Ng;
		protected:
			BSDFImpl* impl;
		public:
			BSDF(BSDFImpl* impl,
				const Vec3f& Ng,
				const CoordinateSystem& localFrame) :impl(impl), Ng(Ng), localFrame(localFrame) {}
			void sample(
				BSDFSample& sample
			)const;

			// evaluate bsdf according to wo, wi
			Spectrum evaluate(
				const Vec3f& wo,
				const Vec3f& wi,
				BSDFSampleOption option,
				BSDFLobe lobe = BSDFLobe::EAll
			)const;

			// evaluate pdf according to wo, wi
			Float evaluatePdf(
				const Vec3f& wo,
				const Vec3f& wi,
				BSDFSampleOption option,
				BSDFLobe lobe = BSDFLobe::EAll
			)const;

			Vec3f localToWorld(const Vec3f& w) const {
				return localFrame.localToWorld(w);
			}

			Vec3f worldToLocal(const Vec3f& w) const {
				return localFrame.worldToLocal(w);
			}
		};
	}
}