#pragma once 
#include <miyuki.h>

namespace Miyuki {
	namespace Core {
		enum AOVType {
			EDiffuseDirect,
			EDiffuseIndirect,
			EGlossyDirect,
			EGlossyIndirect,
			ESpecularAndTransmission,
			AOVCount
		};
		struct AOVRecord {
			Point2f pFilm;
			Spectrum aovs[AOVCount];
			Vec3f normal;
			Spectrum albedo;
			Spectrum L()const {
				Spectrum color;
				for (const auto& i : aovs) {
					color += i;
				}
				return color;
			}
			AOVRecord(const Point2f& pFilm) :pFilm(pFilm) {}
		};
	}
}