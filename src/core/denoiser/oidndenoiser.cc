#include <core/denoiser/oidndenoiser.h>
#include <utils/log.h>
namespace Miyuki {
	namespace Core {
		class OIDNDenoiserImpl {
		public:
			oidn::DeviceRef device;
			IO::Image  color;
			IO::Image  normal, albedo;
			IO::GenericImage<Float> weight;
			oidn::FilterRef filter;
			Point2i dim;
			OIDNDenoiserImpl(const Point2i& dimension)
				:dim(dimension), color(dimension[0], dimension[1]),
				normal(dimension[0], dimension[1]), albedo(dimension[0], dimension[1]),
				weight(dimension[0], dimension[1]) {
				device = oidn::newDevice();
			}
			void addSample(const Point2f& p, const AOVRecord& sample, Float weight = 1.0f) {
				int x = clamp(lroundf(p.x), 0, dim[0] - 1);
				int y = clamp(lroundf(p.y), 0, dim[1] - 1);
				color(x, y) += sample.L();
				normal(x, y) += sample.normal;
				albedo(x, y) += sample.albedo;
				this->weight(x, y) += weight;

			}

			bool denoise(IO::Image& image) {
				image = IO::Image(dim[0], dim[1]);
				for (int i = 0; i < dim[0] * dim[1]; i++) {
					if (weight.pixelData[i] != 0) {
						color.pixelData[i] /= weight.pixelData[i];
						normal.pixelData[i] /= weight.pixelData[i];
						albedo.pixelData[i] /= weight.pixelData[i];
					}
				}
				device.commit();
				filter = oidnNewFilter(device.getHandle(), "RT");
				filter.setImage("color", &color.pixelData[0], oidn::Format::Float3, dim[0], dim[1],0,sizeof(Spectrum));
				filter.setImage("albedo", &albedo.pixelData[0], oidn::Format::Float3, dim[0], dim[1], 0, sizeof(Spectrum));
				filter.setImage("normal", &normal.pixelData[0], oidn::Format::Float3, dim[0], dim[1], 0, sizeof(Spectrum));
				filter.setImage("output", &image.pixelData[0], oidn::Format::Float3, dim[0], dim[1], 0, sizeof(Spectrum));

				filter.commit();
				filter.execute();

				const char* errorMessage;
				if (oidnGetDeviceError(device.getHandle(), &errorMessage) != OIDN_ERROR_NONE)
					Log::log("Error: {}\n", errorMessage);
				return true;
			}
		};
		OIDNDenoiser::OIDNDenoiser() {

		}
		void OIDNDenoiser::setup(const Point2i& dimension) {
			impl = std::make_shared<OIDNDenoiserImpl>(dimension);
		}
		void OIDNDenoiser::addSample(const Point2f& pixel, const AOVRecord& sample, Float weight) {
			impl->addSample(pixel, sample, weight);
		}
		bool OIDNDenoiser::denoise(IO::Image& image) {
			return impl->denoise(image);
		}
	}
}