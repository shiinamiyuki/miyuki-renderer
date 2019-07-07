#include <ui/mainwindow.h>
#include <core/samplers/sampler.h>
//#include <reflection.h>
//#include <graph/materialnode.h>
//#include <io/importobj.h>
//#include <utils/future.hpp>

int main(int argc, char** argv) {
	using namespace Miyuki;
	using namespace Miyuki::Reflection;
	using namespace Core;
	Miyuki::GUI::MainWindow window(argc, argv);
	window.show();

	return 0;
}
