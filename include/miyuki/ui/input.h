#ifndef MIYUKI_INPUT_H
#define MIYUKI_INPUT_H
#include <miyuki.h>
#include <math/transform.h>

namespace Miyuki {
	namespace GUI {
		std::optional<Transform> GetInput(const std::string&, const Transform& initial);
		std::optional<bool> GetInput(const std::string&, bool initial);
		std::optional<int> GetInput(const std::string&, int initial);
		std::optional<Float> GetInput(const std::string&, Float initial);
		std::optional<Vec3f> GetInput(const std::string&, Vec3f initial);
		std::optional<Spectrum> GetInput(const  std::string&, Spectrum initial);
		std::optional<Float> GetFloatClamped(const std::string&, Float initial, Float minVal, Float maxVal);
		std::optional<std::string> GetInput(const std::string&, const std::string&);
	
	}
}
#endif