#include <ui/mainwindow.h>
#include <math/distribution2d.h>
#include <core/rng.h>
#include <core/profile.h>
#include <utils/thread.h>
#include <ppl.h>
constexpr size_t N = 4096;
double A[N][N];
double B[N][N];
double C[N][N];
int main(int argc, char** argv) {
	using namespace Miyuki;
	using namespace Miyuki::Reflection;
	GUI::MainWindow window(argc, argv);
	window.show();


	return 0;
}
