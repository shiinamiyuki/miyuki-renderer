#include <core/cameras/camera.h>
namespace Miyuki {
	namespace Core {
		void PerspectiveCamera::preprocess() {
			computeTransformMatrix();
		}
		void PerspectiveCamera::computeTransformMatrix() {
			rotationMatrix = Matrix4x4::rotation(Vec3f(0, 0, 1), direction.z());
			rotationMatrix = rotationMatrix.mult(Matrix4x4::rotation(Vec3f(0, 1, 0), direction.x()));
			rotationMatrix = rotationMatrix.mult(Matrix4x4::rotation(Vec3f(1, 0, 0), -direction.y()));
			Matrix4x4::inverse(rotationMatrix, invMatrix);
		}
	}
}