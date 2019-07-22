#include <ui/mainwindow.h>

template<size_t>
struct GetBase {
	static const int hasBase = false;
};

int main(int argc, char** argv) {
	using namespace Miyuki;
	using namespace Miyuki::Reflection;
	GUI::MainWindow window(argc, argv);
	window.show();

	return 0;
}
