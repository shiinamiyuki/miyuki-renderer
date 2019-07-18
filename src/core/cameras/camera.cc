#include <core/cameras/camera.h>
#include <math/sampling.h>


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
		Float PerspectiveCamera::generateRay(Sampler& sampler,
			const Point2i& raster,
			Ray* ray,
			CameraSample* sample) {
			Float x = raster.x();
			Float y = raster.y();
			Float rx = 0.5f * (2 * sampler.get1D() - 1);
			Float ry = 0.5f * (2 * sampler.get1D() - 1);
			x += rx;
			y += ry;
			sample->pFilm = Point2f(x, y);
			x = -(2 * x / dimension.x() - 1) * static_cast<Float>(dimension.x()) /
				dimension.y();
			y = 2 * (1 - y / dimension.y()) - 1;

			Vec3f ro(0, 0, 0);
			auto z = (Float)(2.0 / std::tan(fov / 2));
			sample->weight = 1;
			Vec3f rd = Vec3f(x, y, 0) - Vec3f(0, 0, -z);
			rd.normalize();
			if (lensRadius > 0) {
				Point2f pLens = Point2f(lensRadius, lensRadius) * ConcentricSampleDisk(sampler.get2D());
				Float ft = focalDistance / rd.z();
				auto pFocus = ro + ft * rd;
				ro = Vec3f(pLens.x(), pLens.y(), 0);
				rd = (pFocus - ro).normalized();
				sample->pLens = pLens;
			}
			else {
				sample->pLens = {};
			}
			ro = cameraToWorld(ro);
			ro += viewpoint;

			rd = cameraToWorld(rd).normalized();
			*ray = Ray(ro, rd);
			return 1;
		}

		Arc<Film> PerspectiveCamera::createFilm()const {
			return std::make_shared<Film>(dimension.x(), dimension.y());
		}
		Box<Camera> PerspectiveCamera::scale(Float k)const{
			auto camera = new PerspectiveCamera();
			*camera = *this;
			camera->dimension *= k;
			Box<Camera> box;
			box.reset(camera);
			return std::move(box);
		}
	}
}