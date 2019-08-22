//
// Created by Shiina Miyuki on 2019/2/28.
//

#ifndef MIYUKI_SPECTRUM_H
#define MIYUKI_SPECTRUM_H

#include <math/vec.hpp>


namespace Miyuki {
    template<size_t N>
    class CoefficientSpectrum : public Vec<Float, N> {
    public:

    };

    template<>
    class CoefficientSpectrum<3> : public Vec3f {
		using Vec3f::Vec3f;
    };

    class RGBSpectrum : public CoefficientSpectrum<3> {

    public:
		using CoefficientSpectrum::CoefficientSpectrum;
        RGBSpectrum(const Vec3f &c) {
            v[0] = c[0];
            v[1] = c[1];
            v[2] = c[2];
			v[3] = 1;
        }

        RGBSpectrum(const Vec<Float, 3> &c) {
            v[0] = c[0];
            v[1] = c[1];
            v[2] = c[2];
			v[3] = 1;
        }

        RGBSpectrum(Float x = 0) {
            v[0] = x;
            v[1] = x;
            v[2] = x;
			v[3] = 1;
        }

        RGBSpectrum(Float x, Float y, Float z) {
            v[0] = x;
            v[1] = y;
            v[2] = z;
			v[3] = 1;
        }
		RGBSpectrum(Float x, Float y, Float z,Float w) {
			v[0] = x;
			v[1] = y;
			v[2] = z;
			v[3] = w;
		}
        RGBSpectrum &operator=(const Vec3f &c) {
            v[0] = c[0];
            v[1] = c[1];
            v[2] = c[2];
            return *this;
        }

        RGBSpectrum &operator=(const Vec<Float, 3> &c) {
            v[0] = c[0];
            v[1] = c[1];
            v[2] = c[2];
            return *this;
        }

        // performs gamma correction and maps the output to [0, 255)
        RGBSpectrum toInt() const;

        bool hasNaNs() const;

        bool nonNeg() const {
            return r >= 0 && g >= 0 && b >= 0;
        }

        bool isBlack() const;

        inline Float luminance() const {
            return Float(0.299) * r + Float(0.587) * g + Float(0.114) * b;
        }

		RGBSpectrum gamma(Float g=1.0/2.2f)const{
			return RGBSpectrum(std::pow(r, g), std::pow(this->g, g), std::pow(b, g));
		}
    };

    RGBSpectrum removeNaNs(const RGBSpectrum &);

    using Spectrum = RGBSpectrum;

    inline Spectrum clampRadiance(const Spectrum &s, Float maxR) {
        return Spectrum(clamp<Float>(s.r, 0, maxR), clamp<Float>(s.g, 0, maxR), clamp<Float>(s.b, 0, maxR));
    }

	inline void to_json(json& j, const Spectrum& v) {
		j = json{
			v[0],v[1],v[2]
		};
	}

	inline void from_json(const json& j, Spectrum& v) {
		if (j.is_array()) {
			v[0] = j.at(0).get<Float>();
			v[1] = j.at(1).get<Float>();
			v[2] = j.at(2).get<Float>();
		}
		else if(j.is_string()){
			auto s = j.get<std::string>();
			auto i = std::stoi(s,nullptr,16);
			v[2] = i & 0xff;
			v[1] = (i & 0xff00) >> 8;
			v[0] = (i & 0xff0000) >> 16;
			v /= 255.0;
		}
	}
}
#endif //MIYUKI_SPECTRUM_H
