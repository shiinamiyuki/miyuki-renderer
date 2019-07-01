#include <ui/mainwindow.h>

//#include <reflection.h>
//#include <graph/materialnode.h>
//#include <io/importobj.h>
//#include <utils/future.hpp>


namespace Miyuki {

	struct Foo : Miyuki::Trait {
		MYK_IMPL(Foo);
	private:
		int a;
		int b;
	public:
		Foo(int a, int b) :a(a), b(b) {}
		std::vector<int> arr;
		Foo() {}
	};



	struct Bar : Miyuki::Trait {
		MYK_IMPL(Bar);
		int a;
		int b;
		Miyuki::Reflection::Box<Foo> foo;
		Foo* p;
	};
}

MYK_REFL(Miyuki::Bar, (a)(b)(foo)(p))
MYK_REFL(Miyuki::Foo, (a)(b))





#define FOO(...)int A## __VA_ARGS__
int main(int argc, char** argv) {
	using namespace Miyuki;
	using namespace Miyuki::Reflection;
	/*Miyuki::GUI::MainWindow window(argc, argv);
	window.show();*/
	Foo foo(1, 2);
	OutStream s;
	foo.serialize(s);
	std::cout << s.data.dump() << std::endl;
	Box<Trait> p = make_box<Foo>(1, 2);
	std::cout << Foo::type()->name() << std::endl;
	return 0;
}
