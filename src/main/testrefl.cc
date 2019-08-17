#include <reflection.h>
using namespace Miyuki;

MYK_REFL_IMPLEMENTATION;

struct A : Reflective{
	MYK_CLASS(A);
	int a;
	int b;
};
MYK_IMPL(A, "A");
MYK_REFL(A, MYK_NIL_BASE, (a)(b));

int main() {

}