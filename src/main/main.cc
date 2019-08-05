#include <ui/mainwindow.h>
#include <math/distribution2d.h>
#include <core/rng.h>

int main(int argc, char** argv) {
	using namespace Miyuki;
	using namespace Miyuki::Reflection;
	GUI::MainWindow window(argc, argv);
	window.show();
	/*std::vector<Float> v = {
		1,1,1,1,1,1,
		1,1,1,100,1,1,
		1,1,1,1,1,1,
		1,1,1,1,1,1
	};
	Distribution2D dist(&v[0], 6, 4);
	Core::RNG rng(0);
	for (int i = 0; i < 100; i++) {
		Float pdf;
		auto uv = dist.sampleContinuous(Point2f(rng.uniformFloat(), rng.uniformFloat()), &pdf);
		fmt::print("{} {}   {}\n", uv[0], uv[1], pdf);
	}*/
	return 0;
}
