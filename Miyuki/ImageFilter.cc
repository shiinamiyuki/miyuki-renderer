#include "ImageFilter.h"
using namespace Miyuki;
void Miyuki::NonLocalMeanFilter::filter(vec3 *image, int w, int h)
{
	vec3 * in = new vec3[w * h];
	vec3 * buffer = new vec3[w *h];
	for (int i = 0; i < w*h; i++) {
		in[i] = image[i];
	}
	int R = 10;
	double t = 1.0;
	ImageSampler v(in, w, h), u(image, w, h),B(buffer,w,h);
	parallelFor(0u, (unsigned int)w, [&](unsigned int i) {
		for (int j = 0; j < h; j++) {
			vec3 acc(0, 0, 0);
			for (int dx = -R; dx <= R; dx++) {
				for (int dy = -R; dy <= R; dy++) {
					int x = i + dx;
					int y = j + dy;
					acc += B(x, y);
				}
			}
			acc /= (R + 1)*(R + 1);
			B(i, j) = acc;
		}
	});
	parallelFor(0u, (unsigned int)w, [&](unsigned int i) {
		for (int j = 0; j < h; j++){
			vec3 acc = vec3(0,0,0),C=vec3(0,0,0);

			for (int dx = -R; dx <= R; dx++) {
				for (int dy = -R; dy <= R; dy++) {
					int x = i + dx;
					int y = j + dy;
					auto diff = (B(i, j) - B(x, y))/t;
					vec3 f(0, 0, 0);
					f.x() = exp(-pow(diff.x(), 2));
					f.y() = exp(-pow(diff.y(), 2));
					f.z() = exp(-pow(diff.z(), 2));
					acc += f * v(i, j);
					C += f;
				}
			}
			u(i, j) = acc / C;

		}
	});
	delete[] in;
	delete[] buffer;
}
