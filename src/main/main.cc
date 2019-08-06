#include <ui/mainwindow.h>
#include <math/distribution2d.h>
#include <core/rng.h>
#include <kernel/kernel.h>

void test() {
	using namespace Miyuki;
	using namespace Miyuki::Kernel;

	MixedShader shader;
	Float3Shader A, B;
	FloatShader fraction;
	create_mixed_shader(&shader);
	create_float_shader(&fraction);
	fraction.value = 0.5f;
	create_float3_shader(&A);
	create_float3_shader(&B);
	A.value = float3(1, 1, 1);
	B.value = float3(0.5, 0, 0);
	ShadingPoint sp;
	sp.uv = Point2f(1, 1);
	shader.fraction = (Shader*)& fraction;
	shader.shaderA = (Shader*)& A;
	shader.shaderB = (Shader*)& B;
	auto r = shader_eval((Shader*)& shader, &sp);
	fmt::print("{} {} {}\n", r.x, r.y, r.z);
	MYK_KERNEL_PANIC("");
}

int main(int argc, char** argv) {
	using namespace Miyuki;
	using namespace Miyuki::Reflection;
	//GUI::MainWindow window(argc, argv);
	//window.show();
	test();
	return 0;
}
