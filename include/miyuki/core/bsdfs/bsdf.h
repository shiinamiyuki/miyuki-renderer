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
			BSDFLobe lobe = ENone; //sampled lobe
			BSDFSampleOption option;
			BSDFSample():option(ENoSampleOption) {}
			BSDFSample(Intersection& isct, Sampler* sampler, BSDFSampleOption opt)
				:option(opt), uPick(sampler->get1D()) {
				wo = isct.worldToLocal(isct.wo);
				u = sampler->get2D();
			}
		};
		class BSDF;
		struct BSDFEvaluationContext;
		

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
				BSDFEvaluationContext& ctx,
				BSDFSample& sample
			)const = 0;
			static Spectrum evaluate(BSDFImpl * bsdf, const BSDFEvaluationContext& ctx);
			static Float evaluatePdf(BSDFImpl* bsdf, const BSDFEvaluationContext& ctx);
		protected:
			// evaluate bsdf according to wo, wi
			virtual Spectrum evaluate(
				const BSDFEvaluationContext& ctx
			)const = 0;

			// evaluate pdf according to wo, wi
			virtual Float evaluatePdf(
				const BSDFEvaluationContext& ctx
			)const = 0;

		};
		BSDFImpl* getDefaultBSDFImpl();


		class BSDF {
			CoordinateSystem localFrame;
			Vec3f Ng;
		protected:
			BSDFImpl* impl;
		public:
			BSDF(BSDFImpl* impl,
				const Vec3f& Ng,
				const CoordinateSystem& localFrame) :impl(impl), Ng(Ng), localFrame(localFrame) {
				if (!impl) {
					this->impl = getDefaultBSDFImpl();
				}
			}
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
			const Vec3f& getNg()const {
				return Ng;
			}
		};

		struct BSDFEvaluationContext {
			const BSDF& bsdf;			
			BSDFLobe lobe = EAll;
			BSDFSampleOption option = ENoSampleOption;
			BSDFEvaluationContext(
				const BSDF& bsdf,
				const Vec3f &wo,
				BSDFLobe lobe,
				BSDFSampleOption option)
				:bsdf(bsdf), lobe(lobe), option(option)  {
				_wo = wo;
				_woW = bsdf.localToWorld(wo);
			}

			const Vec3f& wi()const {
				return _wi;
			}
			const Vec3f& wo()const {
				return _wo;
			}
			const Vec3f& wiW()const {
				CHECK(assigned);
				return _wiW;
			}
			const Vec3f& woW()const {
				CHECK(assigned);
				return _woW;
			}
			const Vec3f& Ng()const {
				return bsdf.getNg();
			}
			void assignWi(const Vec3f& wi) {
				CHECK(!assigned);
				_wi = wi;
				_wiW = bsdf.localToWorld(wi);
				assigned = true;
			}
			bool isAssigned()const { return assigned; }
		private:
			Vec3f _wo, _wi;
			Vec3f _woW, _wiW;
			bool assigned = false;
		};
	}
}