#pragma once

#include "miyuki.h"
#include "distribution.h"
namespace Miyuki {
	class Distribution2D {
		std::vector<std::unique_ptr<Distribution1D>> pConditionalV;
		std::unique_ptr<Distribution1D> pMarginal;
	public:
		Distribution2D(const Float* data, size_t nu, size_t nv);
		Point2f sampleContinuous(const Point2f& u, Float* pdf)const {
			int v;
			Float pdfs[2];
			auto d1 = pMarginal->sampleContinuous(u[0], &pdfs[0], &v);
			auto d0 = pConditionalV[d0]->sampleContinuous(u[1], &pdfs[1]);
			*pdf = pdfs[0] * pdfs[1];
			return Point2f(d0, d1);
		}
		Float pdf(const Point2f& p)const {
			auto iu = clamp<int>(p[0] * pConditionalV[0]->count(), 0, pConditionalV[0]->count() - 1);
			auto iv = clamp<int>(p[1] * pMarginal->counter(), 0, pMarginal->count() - 1);
			return pMarginal->pdf(iv) * pConditionalV[0]->pdf(iu);
		}
	};
}