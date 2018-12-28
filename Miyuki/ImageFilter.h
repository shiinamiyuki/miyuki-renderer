#pragma once
#include "Miyuki.h"
#include "vec.h"
namespace Miyuki {
	struct ImageSampler {
		vec3 * image;
		int w, h;
		ImageSampler(vec3 *i, int _w, int _h) :w(_w), h(_h), image(i) {}
		vec3& operator () (int x, int y) {
			x = x < 0 ? 0 : x;
			x = x >= w ? w - 1 : x;
			y = y < 0 ? 0 : y;
			y = y >= h ? h - 1 : y;
			return image[x + y * w];
		}
	};

	class Filter
	{
	public:
		virtual void filter(vec3 *, int w, int h) = 0;
		virtual ~Filter() {}
	};
	class NonLocalMeanFilter : public Filter {
	public:
		void filter(vec3 *, int w, int h)override;
	};
}

