#include <ui/mainwindow.h>

int main(int argc, char** argv) {
	using namespace Miyuki;
	using namespace Core;

	Miyuki::GUI::MainWindow window(argc, argv);
	window.show();

	return 0;
}
