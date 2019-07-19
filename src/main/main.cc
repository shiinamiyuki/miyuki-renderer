#include <miyuki.h>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <ui/mainwindow.h>
#include <trait.hpp>

struct A {
	virtual void f() = 0;
};
struct B {
	virtual void g() = 0;
};
struct C:A,B {
	void f() { fmt::print("f\n"); }
	void g() { fmt::print("g\n"); }
	~C() {
		fmt::print("destoryed\n");
	}
};

struct unique_ptr_void_deleter {


};
int main(int argc, char** argv) {
	using namespace Miyuki;
	using namespace Miyuki::Reflection;

	/*Miyuki::GUI::MainWindow window(argc, argv);
	window.show();*/
	/*Trait<A, B> trait(new D());
	foo(trait);*/
	BoxedTrait<A, B> trait;
	trait.reset(new C());
	trait->get<B>()->g();
	BoxedTrait<A> a = std::move(trait);
	a->get<A>()->f();
	return 0;
}
