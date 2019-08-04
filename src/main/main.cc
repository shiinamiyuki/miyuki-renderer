#include <ui/mainwindow.h>
#include <math/distribution.h>
#include <core/rng.h>

int main(int argc, char** argv) {
	using namespace Miyuki;
	using namespace Miyuki::Reflection;
	//GUI::MainWindow window(argc, argv);
	//window.show();
	std::vector<Float> v = { 2,0,4,1,8 };
	Distribution1D dist(&v[0], v.size());
	Core::RNG rng(0);
	std::vector<int> histo(5);
	for (int i = 0; i < 1000; i++) {
		int k = dist.sampleContinuous(rng.uniformFloat()) * 5;
		histo[k]++;
	}
	for (int i = 0; i < 5; i++) {
		fmt::print("{} {}\n", i, histo[i]);
	}
	for (int i = 0; i < 5; i++) {
		fmt::print("pdf({}) = {}\n", i, dist.pdf(i));
	}
	return 0;
}
