//
// Created by Shiina Miyuki on 2019/2/28.
//

#include <core/film.h>
#include <hw/texture.h>
#include <filters/filter.h>

namespace Miyuki {
    Spectrum Pixel::toInt() const {
        return eval().toInt();
    }

    void Pixel::add(const Spectrum &c, const Float &w) {
        value += c;
        filterWeightSum += w;
    }

    Spectrum Pixel::eval() const {
        auto w = filterWeightSum == 0 ? 1 : filterWeightSum;
        auto c = value;
        c /= w;
        return c + Spectrum(splatXYZ[0], splatXYZ[1], splatXYZ[2]) * splatWeight;
    }

    Pixel &Film::getPixel(const Point2f &p) {
        return getPixel(std::lround(p.x()), std::lround(p.y()));
    }

    Pixel &Film::getPixel(int32_t x, int32_t y) {
        x = clamp<int32_t>(x, 0, imageBound.pMax.x() - 1);
        y = clamp<int32_t>(y, 0, imageBound.pMax.y() - 1);
        return image[x + width() * y];
    }

    Pixel &Film::getPixel(const Point2i &p) {
        return getPixel(p.x(), p.y());
    }

    Film::Film(int32_t w, int32_t h)
            : imageBound(Point2i({0, 0}), Point2i({w, h})) {
        assert(w >= 0 && h >= 0);
        image.resize(w * h);
        filter = std::make_unique<MitchellFilter>(Point2f{1.0f, 1.0f}, 1.0 / 3, 1.0 / 3);
        int offset = 0;
        for (int y = 0; y < FilterTableWidth; y++) {
            for (int x = 0; x < FilterTableWidth; x++) {
                Point2f p;
                p.x() = (x + 0.5f) * filter->radius.x() / FilterTableWidth;
                p.y() = (y + 0.5f) * filter->radius.y() / FilterTableWidth;
                filterTable[offset++] = filter->eval(p);
            }
        }
    }

    void Film::writePNG(const std::string &filename) {
        std::vector<unsigned char> pixelBuffer;
        for (const auto &i:image) {
            auto out = i.toInt();
            pixelBuffer.emplace_back(out.r());
            pixelBuffer.emplace_back(out.g());
            pixelBuffer.emplace_back(out.b());
            pixelBuffer.emplace_back(255);
        }
        lodepng::encode(filename, pixelBuffer, (uint32_t) width(), (uint32_t) height());
    }

    void Film::addSample(const Point2f &pos, const Spectrum &c, Float weight) {
        getPixel(pos).add(Spectrum(c * weight), weight);
		if (hwBuffer) {
			hwBuffer->setPixel(pos, getPixel(pos).eval());
		}
    }

    void Film::clear() {
        for (auto &i:image) {
            i.value = Spectrum{};
            i.filterWeightSum = 0;
        }
    }

    Bound2i
    Intersect(const Bound2i &b1, const Bound2i &b2) {
        return Bound2i(Point2i(std::max(b1.pMin.x(), b2.pMin.x()),
                               std::max(b1.pMin.y(), b2.pMin.y())),
                       Point2i(std::min(b1.pMax.x(), b2.pMax.x()),
                               std::min(b1.pMax.y(), b2.pMax.y())));
    }

    std::unique_ptr<FilmTile> Film::getFilmTile(const Bound2i &bounds) {
        auto floatBounds = (Bound2f) bounds;
        Point2i p0 = (Point2i) Ceil(floatBounds.pMin -
                                    filter->radius);
        Point2i p1 = (Point2i) Floor(floatBounds.pMax +
                                     filter->radius) + Point2i(1, 1);
        Bound2i tilePixelBounds =
                Intersect(Bound2i(p0, p1), imageBound);
        return std::move(std::unique_ptr<FilmTile>(new FilmTile(bounds, filterTable, filter.get())));
    }

    void Film::mergeFilmTile(const FilmTile &tile) {
        std::lock_guard<std::mutex> lockGuard(lock);
        for (int x = tile.pixelBounds.pMin.x(); x < tile.pixelBounds.pMax.x(); x++) {
            for (int y = tile.pixelBounds.pMin.y(); y < tile.pixelBounds.pMax.y(); y++) {
                getPixel(x, y).value += tile.getPixel({x, y}).value;
                getPixel(x, y).filterWeightSum += tile.getPixel({x, y}).filterWeightSum;
            }
        }
    }

    void Film::enableAuxBuffer() {
        auxBuffer = std::make_unique<IO::GenericImage<LighingComposition>>(width(), height());
    }

    void Film::addSample(const Point2f &p, const LighingComposition &context) {
        if (auxBuffer) {
            int x = clamp(lroundf(p.x()), 0, width() - 1);
            int y = clamp(lroundf(p.y()), 0, height() - 1);
            auto &pixel = (*auxBuffer)(x, y);
            pixel.combineSamples(context);
            pixel.color.variance.addSample(context.color.eval());
            pixel.depth.variance.addSample(context.depth.eval());
            pixel.normal.variance.addSample(context.normal.eval());
            pixel.albedo.variance.addSample(context.albedo.eval());
            pixel.direct.variance.addSample(context.direct.eval());
            pixel.indirect.variance.addSample(context.indirect.eval());
        }
    }

    FilmTile::FilmTile(const Bound2i &bound2i, const Float *filterTable, const Filter *filter)
            : pixelBounds(bound2i), filterTable(filterTable), filter(filter) {
        int area = (pixelBounds.pMax.x() - pixelBounds.pMin.x())
                   * (pixelBounds.pMax.y() - pixelBounds.pMin.y());
        pixels.resize(area);
    }

    void FilmTile::addSample(const Point2f &raster, const Spectrum &sample, Float weight) {
        auto p = raster - Point2f(pixelBounds.pMin);
        Point2i p0 = (Point2i) Ceil(raster - filter->radius);
        Point2i p1 = (Point2i) Floor(raster + filter->radius) + Point2i(1, 1);
        p0 = Max(p0, pixelBounds.pMin);
        p1 = Min(p1, pixelBounds.pMax);
        for (int y = p0.y(); y < p1.y(); ++y) {
            for (int x = p0.x(); x < p1.x(); ++x) {
                auto pos = Point2i(x, y);
                auto &pixel = getPixel(pos);
                auto offset = raster - Point2f(pos);
                offset /= filter->radius;
                offset = Point2f(std::abs(offset[0]), std::abs(offset[1]));
                offset *= FilterTableWidth;
                int i = clamp<int>(std::floor(offset.x())
                                   + FilterTableWidth * std::floor(offset.y()), 0,
                                   FilterTableWidth * FilterTableWidth - 1);
                auto filterWeight = filterTable[i];
                pixel.filterWeightSum += filterWeight;
                pixel.value += sample * weight * filterWeight;

            }
        }

    }

    TilePixel &FilmTile::getPixel(const Point2i &raster) {
        auto p = raster - pixelBounds.pMin;
        int i = clamp<int>(std::lround(p.x()) + TileSize * std::lround(p.y()), 0, pixels.size() - 1);
        return pixels[i];
    }

    const TilePixel &FilmTile::getPixel(const Point2i &raster) const {
        auto p = raster - pixelBounds.pMin;
        int i = clamp<int>(std::lround(p.x()) + TileSize * std::lround(p.y()), 0, pixels.size() - 1);
        return pixels[i];
    }
}