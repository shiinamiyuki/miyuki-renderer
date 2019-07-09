#include <ui/mainwindow.h>

struct Flyable {
	void fly() {
		std::cout << "fly" << std::endl;
	}
};



MYK_TRAIT(Flyable, (fly))

struct HighFlyable : Flyable {
	void flyHigh() {}
};

MYK_TRAIT_DERIVED(HighFlyable, (Flyable),(flyHigh))

struct Foo {
	void fly() {
		std::cout << "foo fly" << std::endl;
	}
	void flyHigh() {
		std::cout << "foo flyhigh"  << a << std::endl;
	}
	Foo(int a) :a(a) {}
	int a;
};


int main(int argc, char** argv) {
	using namespace Miyuki;
	using namespace Miyuki::Reflection;
	using namespace Core;


	AnyPtr<HighFlyable> ptr(new Foo(3));
	ptr->flyHigh();
	/*Miyuki::GUI::MainWindow window(argc, argv);
window.show();*/
//auto f = toLambda(&Flyable::fly);
	return 0;
}
