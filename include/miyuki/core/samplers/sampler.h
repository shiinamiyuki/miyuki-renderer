#ifndef MIYUKI_SAMPLER_H
#define MIYUKI_SAMPLER_H

#include <reflection.h>
#include <core/rng.h>

namespace Miyuki {
	namespace Core {
		
		struct SamplerState {
			Point2i resolution;
			Point2i pixel;
			uint32_t sample;
			uint32_t samplesPerPixel;
			SamplerState() {}
			SamplerState(const Point2i& resolution,
				const Point2i& pixel,
				uint32_t sample = 0,
				uint32_t samplesPerPixel = 0xffffff)
			:resolution(resolution),
			sample(sample),pixel(pixel),samplesPerPixel(samplesPerPixel){}
		};

		class Sampler : public Reflective{
		public:
			MYK_ABSTRACT(Sampler);
			virtual Box<Sampler> clone()const = 0;
			virtual Float get1D() = 0;
			virtual Point2f get2D() = 0;
			virtual void start(const SamplerState&) = 0;
			virtual bool startNextSample() = 0;
			SamplerState& getState() {
				return state;
			}
			const SamplerState& getState()const {
				return state;
			}
		protected:
			SamplerState state;

		};
		MYK_REFL(Sampler, (Reflective), MYK_REFL_NIL);

		class RandomSampler final : public Sampler {
		private:
			RNG rng;
		public:
			MYK_CLASS(RandomSampler);
			RandomSampler() {}
			RandomSampler(RNG rng, const SamplerState& state) :rng(std::move(rng)){
				this->state = state;
			}
			virtual Box<Sampler> clone()const override {
				auto sampler = Reflection::makeBox<RandomSampler>(rng, state);
				return std::move(sampler);
			}
			virtual Float get1D()override {
				return rng.uniformFloat();
			}
			virtual Point2f get2D() override {
				return { get1D(), get1D() };
			}
			virtual void start(const SamplerState& state) override {
				this->state = state;
			}
			virtual bool startNextSample()override {
				state.sample++;
				return state.sample < state.samplesPerPixel; 
			}
		};
		MYK_IMPL(RandomSampler,  "Sampler.Random");
		MYK_REFL(RandomSampler, (Sampler), MYK_REFL_NIL);

		class SobolSampler final: public Sampler {
			uint32_t dimension = 0;
			RNG rng;
			uint32_t rotation = 0;
		public:
			MYK_CLASS(SobolSampler);
			SobolSampler() {}
			SobolSampler(const SamplerState& state) {
				rotation = rng.uniformFloat();
				this->state = state;
			}
			virtual Box<Sampler> clone()const override {
				auto sampler = Reflection::makeBox<SobolSampler>(*this);
				return std::move(sampler);
			}
			virtual Point2f get2D() override {
				return { get1D(), get1D() };
			}
			virtual Float get1D()override;

			virtual void start(const SamplerState& state) override {
				this->state = state;
				rng = RNG(0);
				rng.advance(state.pixel[0] + state.pixel[1] * state.resolution[0]);
				rotation = rng.uniformUint32();
			}
			virtual bool startNextSample()override {
				dimension = 0;
				state.sample++;
				return state.sample < state.samplesPerPixel;
			}
		};
		MYK_IMPL(SobolSampler, "Sampler.Sobol");
		MYK_REFL(SobolSampler, (Sampler),  MYK_REFL_NIL);
	}
}

#endif