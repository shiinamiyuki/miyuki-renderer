#include <core/rasterizer/rasterizer.h>

namespace Miyuki {
	namespace Core {
		struct RasterData {
			Spectrum color;
			Vec3f normal;
			Float depth;
		};

		inline int edgeFunction(int x1, int y1, int x2, int y2, int px, int py) {
			return ((px - x1) * (y2 - y1) - (py - y1) * (x2 - x1));
		}

		struct RasterBuffer : IO::Image<RasterData> {
			RasterBuffer(size_t w, size_t h) :IO::Image<RasterData>(w, h) {}
			void drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, vec3 n) {
				int minX = std::min(std::min(x1, x2), x3);
				int maxX = std::max(std::max(x1, x2), x3);
				int minY = std::min(std::min(y1, y2), y3);
				int maxY = std::max(std::max(y1, y2), y3);
				for (int x = std::max(0, minX); x <= std::min(maxX, width); x++) {
					bool everInside = false;
					for (int y = std::max(0, minY); y <= std::min(maxY, height); y++) {
						int w1 = -edgeFunction(x1, y1, x2, y2, x, y);
						int w2 = -edgeFunction(x2, y2, x3, y3, x, y);
						int w3 = -edgeFunction(x3, y3, x1, y1, x, y);
						bool inside = w1 >= 0 && w2 >= 0 && w3 >= 0;
						if (inside) {
							/*color(x, y) = vec3(1);
							normal(x, y) = n;*/
							auto& pixel = (*this)(x, y);
							pixel.color = Vec3f(1);
							pixel.normal = n;

							everInside = true;
						}
						else {
							if (everInside) {
								break;
							}
						}
					}
				}
			}
			Vec3f rasterize(Float fov, const Vec3f& v) {
				auto z = (float)(2.0 / tan(fov / 2));
				return Vec3f(v.x / v.z * z, v.y / v.z * z, 0);
			}

			void rasterize(Float fov, Vec3f v0, Vec3f v1, Vec3f v2) {
				v0 = rasterize(fov, v0);
				v1 = rasterize(fov, v1);
				v2 = rasterize(fov, v2);

				v0.y = -v0.y;
				v1.y = -v1.y;
				v2.y = -v2.y;

				// to screen space
				v0 = (v0 + Vec3f(1)) / 2 * Vec3f(width, height, 1);
				v1 = (v1 + Vec3f(1)) / 2 * Vec3f(width, height, 1);
				v2 = (v2 + Vec3f(1)) / 2 * Vec3f(width, height, 1);

				auto n = Vec3f::cross(v2 - v0, v1 - v0).normalize();
				drawTriangle(v0.x, v0.y, v1.x, v1.y, v2.x, v2.y, n);
			}
		};

		class SoftRasterizer : public Rasterizer {
			Arc<IO::Image<RasterData>> initBuffer(size_t w, size_t h, size_t MSAA);
		public:
			MYK_CLASS(SoftRasterizer);
			virtual void render(RasterizationContext& ctx)override;
		};
		MYK_IMPL(SoftRasterizer, "Rasterizer.Soft");
		MYK_REFL(SoftRasterizer, (Rasterizer), MYK_REFL_NIL);
	}
}