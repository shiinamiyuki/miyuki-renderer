#pragma once
#include <miyuki.h>

namespace Miyuki {
	struct ClampedFloat {
		Float value;
		Float minVal, maxVal;
		void set_value(Float v) {
			value = clamp(v, minVal, maxVal);
		}
		ClampedFloat(Float value, Float minVal = 0.0f, Float maxVal = 1.0f)
			:value(value), minVal(minVal), maxVal(maxVal) {}
	};
}