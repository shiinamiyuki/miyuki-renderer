#include <core/integrators/kpt.h>
#include <core/scene.h>
#include <core/kernelrecord.h>
#include <kernel/kernel_globals.h>
#include <kernel/kernel.h>
#include <kernel/kernel_impl_cpu.h>

namespace Miyuki {
	namespace Core {
		void KernelPathTracer::renderProgressive(
			const IntegratorContext& context,
			ProgressiveRenderCallback progressiveCallback) {
			_aborted = false;
			auto record = context.scene->getKernelRecord();
			Kernel::KernelGlobals globals = record->createKernelGlobals();
			globals.scene = (Kernel::Scene*)context.scene;
			auto match = Reflection::match(context.camera.get())
				.with<Core::PerspectiveCamera>([&](PerspectiveCamera* camera) {
				globals.camera.dimension = context.film->imageDimension();
				globals.camera.direction = camera->direction;
				globals.camera.position = camera->viewpoint;
				globals.camera.fov = camera->fov;
				globals.camera.lens_radius = camera->lensRadius;
				globals.camera.focal_distance = camera->focalDistance;
				auto rotationMatrix = Matrix4x4::rotation(Vec3f(0, 0, 1), camera->direction.z);
				rotationMatrix = rotationMatrix.mult(Matrix4x4::rotation(Vec3f(0, 1, 0), camera->direction.x));
				rotationMatrix = rotationMatrix.mult(Matrix4x4::rotation(Vec3f(1, 0, 0), -camera->direction.y));

				for (int i = 0; i < 4; i++) {
					globals.camera.transform.m[i] = rotationMatrix.m[i];
				}
			});
			Assert(match.matched); 
		}
	}
}