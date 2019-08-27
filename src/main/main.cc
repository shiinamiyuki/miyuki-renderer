#include <ui/mainwindow.h>
#include <utils/thread.h>


int main(int argc, char** argv) {
	using namespace Miyuki;
	using namespace Miyuki::Reflection;
	Init();
	GUI::MainWindow window(argc, argv);
	window.show();
	Exit();
	return 0;
}
