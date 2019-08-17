#include <reflection.h>
using namespace Miyuki;

MYK_REFL_IMPLEMENTATION;

struct A : Reflective {
	MYK_CLASS(A);
	int a;
	int b;
	Arc<A> box;
	Weak<A> p;
};
MYK_IMPL(A, "A");
MYK_REFL(A, MYK_NIL_BASE, (a)(b)(p)(box));

int main() {
	try {
		A a;
		a.a = 2;
		a.b = 4;
		a.box = makeArc<A>();
		a.box->a = 234;
		a.box->p = a.box;
		a.p = a.box;
		Reflection::OutObjectStream stream;
		a.serialize(stream);
		auto dump = stream.toJson().dump(1);
		fmt::print("{}\n", dump);
		auto j = stream.toJson();
		Reflection::InObjectStream in(j);
		A a2;
		a2.deserialize(in);
		fmt::print("doen\n");
	}
	catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
	}
}