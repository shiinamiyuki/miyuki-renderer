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

		struct Sampler : Component{
			MYK_INTERFACE(Sampler);
			virtual std::unique_ptr<Sampler> clone()const = 0;
			virtual Float get1D() = 0;
			virtual Point2f get2D() = 0;
			virtual void start(const SamplerState&) = 0;
			virtual bool startNextSample() = 0;
			virtual SamplerState getState() = 0;
		};
		MYK_EXTENDS(Sampler, (Component));

		struct RandomSampler final : Sampler {
		private:
			RNG rng;
			SamplerState state;
		public:
			MYK_META(RandomSampler);
			RandomSampler() {}
			RandomSampler(RNG rng, SamplerState state) :rng(std::move(rng)), state(std::move(state)) {}
			virtual std::unique_ptr<Sampler> clone()const override {
				auto sampler = std::make_unique<RandomSampler>(rng, state);
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
			virtual SamplerState getState()override {
				return state;
			}
			virtual bool startNextSample()override {
				state.sample++;
				return state.sample < state.samplesPerPixel; 
			}
		};
		MYK_IMPL(RandomSampler, (Sampler), "Sampler.Random");
		MYK_REFL(Miyuki::Core::RandomSampler, MYK_REFL_NIL);
		struct SobolSampler final: Sampler {
			SamplerState state;
			uint32_t dimension = 0;
			RNG rng;
			uint32_t rotation = 0;
		public:
			MYK_META(SobolSampler);
			SobolSampler() {}
			SobolSampler(const SamplerState& state) :state(state) {
				rotation = rng.uniformFloat();
			}
			virtual std::unique_ptr<Sampler> clone()const override {
				auto sampler = std::make_unique<SobolSampler>(*this);
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
			virtual SamplerState getState()override {
				return state;
			}
			virtual bool startNextSample()override {
				dimension = 0;
				state.sample++;
				return state.sample < state.samplesPerPixel;
			}
		};
		MYK_IMPL(SobolSampler, (Sampler), "Sampler.Sobol");
		MYK_REFL(Miyuki::Core::SobolSampler, MYK_REFL_NIL);
	}
}

#endif