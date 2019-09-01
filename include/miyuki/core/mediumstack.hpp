#pragma once

namespace Miyuki {
	namespace Core {
		constexpr size_t MaxNestedMedium = 16;

		class Medium;
		struct MediumStack {
			void push(Medium* m) {
				stack[sp++] = m;
			}
			Medium* pop() {
				return stack[--sp];
			}
			Medium* top()const {
				return stack[sp - 1];
			}
			size_t size()const {
				return sp;
			}
			bool contains(Medium* m)const {
				for (int i = 0; i < sp; i++) {
					if (stack[i] == m)
						return true;
				}
				return false;
			}
		private:
			Medium* stack[MaxNestedMedium];
			int sp = 0;
		};
	}
}