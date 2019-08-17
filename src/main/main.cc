#include <ui/mainwindow.h>
#include <math/distribution2d.h>
#include <core/rng.h>

namespace Miyuki {
	struct Vec3Uint24 {
		uint32_t get(size_t i)const {
			int result = 0;
			const char* p;
			if (i == 0) {
				p = _data;
			}
			else if (i == 1) {
				p = _data + 3;
			}
			else if (i == 2) {
				p = _data + 6;
			}
			else {
				throw std::out_of_range("Vec3Uint24");
			}
			result = p[0] | (p[1] << 8) | (p[2] << 16);
			return result;
		}
		void set(size_t i, uint32_t val) {
			char* p;
			if (i == 0) {
				p = _data;
			}
			else if (i == 1) {
				p = _data + 3;
			}
			else if (i == 2) {
				p = _data + 6;
			}
			else {
				throw std::out_of_range("Vec3Uint24");
			}
			p[0] = val & 0xff;
			p[1] = (val & 0xff00) >> 8;
			p[2] = (char)((val & 0xff0000) >> 16);
		}
		char _data[12] = { 0 };
	};
	static_assert(sizeof(Vec3Uint24) == 12, "Otherwise whats the point");
}
struct A {
	union {
		Miyuki::_Vec3f v;
		int i;
	};
};
int main(int argc, char** argv) {
	using namespace Miyuki;
	using namespace Miyuki::Reflection;
	Init();
	GUI::MainWindow window(argc, argv);
	window.show();
	A a;
	Exit();
	return 0;
}
