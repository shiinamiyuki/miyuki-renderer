//
// Created by Shiina Miyuki on 2019/2/28.
//
#include <miyuki.h>
using namespace Miyuki;
static auto _toInt = [](Float x) -> int {
	return lround(clamp<Float>(x * 255, 0.0f, 255.0));
};

RGBSpectrum RGBSpectrum::toInt() const {
	auto c = gamma();
	return RGBSpectrum(_toInt(c.r), _toInt(c.g), _toInt(c.b));
}

bool RGBSpectrum::hasNaNs() const {
	return std::isnan(r) || std::isnan(g) || std::isnan(b);
}

bool RGBSpectrum::isBlack() const {
	return r <= 0 && g <= 0 && b <= 0;

}

static Float removeNaN(Float x) {
	if (std::isnan(x))return 0;
	return x;
}

RGBSpectrum Miyuki::removeNaNs(const RGBSpectrum& spectrum) {
	return RGBSpectrum(removeNaN(spectrum.r), removeNaN(spectrum.g), removeNaN(spectrum.b));
}
