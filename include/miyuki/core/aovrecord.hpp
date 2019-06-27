#pragma once 
#include <miyuki.h>

namespace Miyuki {
	namespace Core {
		enum AOVType {
			kDiffuseDirect,
			kDiffuseIndirect,
			kGlossyDirect,
			kGlossyIndirect,
			kSpecularAndTransmission,
			kAOVCount
		};
		struct AOVRecord {
			Point2f pFilm;
			Spectrum aovs[kAOVCount];
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