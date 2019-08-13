#include <ui/mainwindow.h>
#include <math/distribution2d.h>
#include <core/rng.h>


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
