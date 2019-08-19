#include <core/denoiser/bcddenoiser.h>

#include <utils/log.h>
#include <bcd/bcd/Denoiser.h>

namespace Miyuki {
	namespace Core {
		

		AOVDenoiser::AOVDenoiser(const Point2i& dimension,
			const DenoiserHistogramParameters& parameters)
			:dimension(dimension), accumulator(dimension[0], dimension[1], parameters) {
		}

		void AOVDenoiser::addSample(const Point2f& p, const Spectrum& color, Float weight) {
			int x = clamp(lroundf(p.x), 0, dimension[0] - 1);
			int y = clamp(lroundf(p.y), 0, dimension[1] - 1);
			accumulator.addSample(y, x, color[0], color[1], color[2], weight);
		}

		bool AOVDenoiser::denoise(IO::Image& image) {
			std::unique_ptr<bcd::IDenoiser> denoiser(new bcd::Denoiser());
			auto stats = accumulator.getSamplesStatistics();
			bcd::DenoiserInputs inputs;
			bcd::DenoiserOutputs output;
			bcd::Deepimf denoisedColor(dimension[0], dimension[1], 3);
			output.m_pDenoisedColors = &denoisedColor;
			inputs.m_pColors = &stats.m_meanImage;
			inputs.m_pHistograms = &stats.m_histoImage;
			inputs.m_pNbOfSamples = &stats.m_nbOfSamplesImage;
			inputs.m_pSampleCovariances = &stats.m_covarImage;
			denoiser->setInputs(inputs);
			denoiser->setOutputs(output);
			if (!denoiser->denoise()) {
				return false;
			}
			output = denoiser->getOutputs();
			image = IO::Image(dimension[0], dimension[1]);
			for (int i = 0; i < dimension[0]; i++) {
				for (int j = 0; j < dimension[1]; j++) {
					image({ i, j })[0] = output.m_pDenoisedColors->get(j, i, 0);
					image({ i, j })[1] = output.m_pDenoisedColors->get(j, i, 1);
					image({ i, j })[2] = output.m_pDenoisedColors->get(j, i, 2);
				}
			}

			return true;
		}
		DenoiserDriver::DenoiserDriver(const Point2i& dimension,
			const DenoiserHistogramParameters& p) :dimension(dimension) {
			for (auto& i : aov) {
				i = std::make_unique<AOVDenoiser>(dimension, p);
			}
		}

		const char* AOVString(AOVType type) {
			switch (type) {
			case EDiffuseDirect:
				return "diffuse direct";
			case EDiffuseIndirect:
				return "diffuse indirect";
			case EGlossyDirect:
				return "glossy direct";
			case EGlossyIndirect:
				return "glossy indirect";
			case ESpecularAndTransmission:
				return "specular";
			}
			return "";
		}
		bool DenoiserDriver::denoise(IO::Image& image) {
			Log::log("Denoising\n");

			IO::Image denoisedAOV[AOVCount];
			image = IO::Image(dimension[0], dimension[1]);
			for (int i = 0; i < AOVCount; i++) {
				denoisedAOV[i] = IO::Image(dimension[0], dimension[1]);
				Log::log("Denoising aov [{}]\n", AOVString((AOVType)i));
				if (!aov[i]->denoise(denoisedAOV[i])) {
					Log::log("Denoising aov [{}] failed\n", AOVString((AOVType)i));
					return false;
				}
				denoisedAOV[i].save(fmt::format("temp/denoised {}.png", AOVString((AOVType)i)));
			}

			for (int i = 0; i < image.pixelData.size(); i++) {
				image[i] = {};
				for (int j = 0; j < AOVCount; j++) {
					image[i] += denoisedAOV[j][i];
				}
			}

			Log::log("Denoising completed\n");
			return true;
		}
		void BCDDenoiser::setup(const Point2i& dim) {
			driver.reset(new DenoiserDriver(dim, DenoiserHistogramParameters()));
		}
		void BCDDenoiser::addSample(const Point2f& pixel, const AOVRecord& sample, Float weight) {
			for (int i = 0; i < AOVCount; i++) {
				driver->addSample((AOVType)i, pixel, sample.aovs[i], weight);
			}
		}
		bool  BCDDenoiser::denoise(IO::Image&image){
			return driver->denoise(image);
		}
	}
}