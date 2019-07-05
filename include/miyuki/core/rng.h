#ifndef MIYUKI_RNG_H
#define MIYUKI_RNG_H

#include <miyuki.h>


namespace Miyuki {
	namespace Core {

		static const Float OneMinusEpsilon = 0x1.fffffep-1;

		// from pbrt
		static const uint64_t PCG32_DEFAULT_STATE = 0x853c49e6748fea9bULL;
		static const uint64_t PCG32_DEFAULT_STREAM = 0xda3e39cb94b95bdbULL;
		static const uint64_t PCG32_MULT = 0x5851f42d4c957f2dULL;

		struct RNG {
			RNG():state(PCG32_DEFAULT_STATE), inc(PCG32_DEFAULT_STREAM) {}
			RNG(uint64_t seq) { setSequence(seq); }
			uint32_t uniformUint32() {
				uint64_t oldstate = state;
				state = oldstate * PCG32_MULT + inc;
				uint32_t xorshifted = (uint32_t)(((oldstate >> 18u) ^ oldstate) >> 27u);
				uint32_t rot = (uint32_t)(oldstate >> 59u);
				return (xorshifted >> rot) | (xorshifted << ((~rot + 1u) & 31));
			}
			Float uniformFloat() {
				return std::min(OneMinusEpsilon, Float(uniformUint32() * 0x1p-32f));
			}
			void setSequence(uint64_t initseq) {
				state = 0u;
				inc = (initseq << 1u) | 1u;
				uniformUint32();
				state += PCG32_DEFAULT_STATE;
				uniformUint32();
			}

			void advance(int64_t idelta) {
				uint64_t cur_mult = PCG32_MULT, cur_plus = inc, acc_mult = 1u,
					acc_plus = 0u, delta = (uint64_t)idelta;
				while (delta > 0) {
					if (delta & 1) {
						acc_mult *= cur_mult;
						acc_plus = acc_plus * cur_mult + cur_plus;
					}
					cur_plus = (cur_mult + 1) * cur_plus;
					cur_mult *= cur_mult;
					delta /= 2;
				}
				state = acc_mult * state + acc_plus;
			}
		private:
			uint64_t state, inc;
		};
	}
}
#endif