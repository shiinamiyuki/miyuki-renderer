//
// Created by Shiina Miyuki on 2019/2/28.
//

#ifndef MIYUKI_FILM_H
#define MIYUKI_FILM_H

#include <miyuki.h>
#include <utils/atomicfloat.h>
#include <filters/filter.h>
#include <io/image.h>


namespace Miyuki {
	namespace HW {
		class Texture;
	}

	namespace Core {
		static constexpr int TileSize = 16;
		static constexpr int FilterTableWidth = 16;

		
		struct Pixel {
			Spectrum value;
			Float filterWeightSum;
			AtomicFloat splatXYZ[3];
			Float splatWeight = 1;

			Pixel() : value(0, 0, 0), filterWeightSum(0) {}

			Spectrum toInt() const;

			Spectrum eval() const;

			void add(const Spectrum& c, const Float& w);
		};

		struct TilePixel {
			Spectrum value;
			Float filterWeightSum = 0;
		};

		struct Film;

		struct FilmTile {
		private:
			friend struct Film;

			std::vector<TilePixel> pixels;
			Bound2i pixelBounds;
			const Float* filterTable = nullptr;
			const Filter* filter;
		public:
			FilmTile(const Bound2i& bound2i, const Float* filterTable, const Filter* filter);

			TilePixel& getPixel(const Point2i&);

			const TilePixel& getPixel(const Point2i&) const;

			void addSample(const Point2f& raster, const Spectrum& sample, Float weight = 1);
		};

		struct Film {
		public:
			std::vector<Pixel> image;
			std::unique_ptr<Filter> filter;
			HW::Texture* hwBuffer = nullptr;
		private:
			Float filterTable[FilterTableWidth * FilterTableWidth];
			Bound2i imageBound;
			std::mutex lock;
		public:

			const Point2i& imageDimension() const { return imageBound.pMax; }

			size_t height() const { return imageBound.pMax.y; }

			size_t width() const { return imageBound.pMax.x; }

			Pixel& getPixel(const Point2f&);

			Pixel& getPixel(const Point2i&);

			Pixel& getPixel(int x, int y);

			void addSample(const Point2f&, const Spectrum& c, Float weight = 1);

			void addSplat(const Point2f& pos, const Spectrum& c) {
				getPixel(pos).splatXYZ[0].add(c[0]);
				getPixel(pos).splatXYZ[1].add(c[1]);
				getPixel(pos).splatXYZ[2].add(c[2]);
			}

			Float& splatWeight(const Point2i& pos) {
				return getPixel(pos).splatWeight;
			}

			Film(int w = 0, int h = 0);
			Film(const Point2i& dim) :Film(dim[0], dim[1]) {}
			void writePNG(const std::string& filename);

			void clear();

			void mergeFilmTile(const FilmTile&);

			std::unique_ptr<FilmTile> getFilmTile(const Bound2i& bounds);
		};
	}
}
#endif //MIYUKI_FILM_H
