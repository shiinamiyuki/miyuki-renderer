#include <ui/mainwindow.h>
#include <reflection.h>
#include <graph/materialnode.h>
#include <io/importobj.h>
#include <utils/future.hpp>

int main(int argc, char** argv) {
	Miyuki::GUI::MainWindow window(argc, argv);
	window.show();
	return 0;
}