#pragma once

#include <math/vec.hpp>

namespace Miyuki {
	namespace Core {

		/*
		*	A generic non-delta pdf 
		*/
		template<
			class ImplT, 
			class DomainT=Vec3f,
			class ValueT=Float,
			class SampleT=Point2f>
		struct PDF {
			DomainT sample(const SampleT& u, SampleT* pdf=nullptr)const {
				auto tmp =  This().sampleImpl(u);
				if (pdf)* pdf = evaluate(tmp);
				return tmp;
			}
			ValueT evaluate(const DomainT& p)const {
				return This().evaluateImpl(p);
			}
		private:
			const ImplT& This()const {
				return *static_cast<const ImplT*>(this);
			}
		};

	}
}