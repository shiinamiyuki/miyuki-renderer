#include <ui/mainwindow.h>
#include <reflection.h>
#include <graph/materialnode.h>
#include <io/importobj.h>
#include <utils/future.hpp>
int main(int argc, char** argv) {
	/*Miyuki::GUI::MainWindow window(argc, argv);
	window.show();*/
	using namespace Miyuki;
	auto f = Async([]() {
		std::this_thread::sleep_for(std::chrono::seconds(3));
		return 1; 
	}).then([=](int i) {
		fmt::print("{}\n", i);
	});
	fmt::print("counting...\n");
	return 0;
}