#ifndef MIYUKI_FRESNEL_HPP
#define MIYUKI_FRESNEL_HPP

#include <core/bsdfs/trignometry.hpp>

namespace Miyuki {
	namespace Core {
		enum FresnelType {
			EPerfectSpecular
		};

		template<class Model>
		struct FresnelFunction {
			Vec3f evaluate(Float cosTheta)const {
				return This().evaluateImpl(cosTheta);
			}
		private:
			const Model& This()const {
				return *static_cast<const Model*>(this);
			}
		};

		struct PerfectSpecularFresnel : FresnelFunction< PerfectSpecularFresnel> {
			Vec3f evaluateImpl(Float cosTheta)const {
				return Vec3f(1);
			}
		};

		struct FresnelWrapper : FresnelFunction<FresnelWrapper> {
			FresnelType type;
			union {
				PerfectSpecularFresnel perfectSpecular;
			};
			FresnelWrapper(FresnelType type):type(type){
				switch (type) {
				case EPerfectSpecular:
					break;		
				}
			}
			Vec3f evaluateImpl(Float cosTheta)const {
				switch (type) {
				case EPerfectSpecular:
					return perfectSpecular.evaluateImpl(cosTheta);
				}
				CHECK(false);
				return {};
			}
		};
	}
}

#endif