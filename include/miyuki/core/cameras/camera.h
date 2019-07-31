#ifndef MIYUKI_CAMERA_H
#define MIYUKI_CAMERA_H

#include <reflection.h>
#include <core/samplers/sampler.h>
#include <core/ray.h>
#include <math/transform.h>
#include <core/film.h>

namespace Miyuki {
	namespace Core {
		struct CameraSample {
			Point2f pFilm, pLens;
			Float weight;
		};

		class Camera : public Reflective {
		public:
			MYK_ABSTRACT(Camera);
			virtual Float generateRay(Sampler& sampler,
				const Point2i& filmDimension,
				const Point2i& raster,
				Ray* ray,
				CameraSample*) = 0;

			virtual Float
				generateRayDifferential(Sampler& sampler, 
					const Point2i& filmDimension,
					const Point2i& raster,
					RayDifferential* ray, Float* weight) = 0;

			virtual void preprocess() {  }	
			virtual Vec4f cameraToWorld(const Vec4f& w) const = 0;
			virtual Vec4f worldToCamera(const Vec4f& w) const = 0;
			virtual Box<Camera> clone()const = 0;

			Vec3f viewpoint;
			// euler angle;
			Vec3f direction;

		};
		MYK_REFL(Camera, (Reflective), (viewpoint)(direction));

		class PerspectiveCamera final: public Camera {
		
			Matrix4x4 rotationMatrix, invMatrix;
		public:
			Float lensRadius, focalDistance;
			Float fov = DegreesToRadians(80.0);
			MYK_CLASS(PerspectiveCamera);
			virtual void preprocess()override;

			virtual Vec4f cameraToWorld(const Vec4f& w) const override {
				return rotationMatrix.mult(w);
			}

			virtual Vec4f worldToCamera(const Vec4f& w) const override {
				return invMatrix.mult(w);
			}
			virtual Float generateRay(Sampler& sampler,
				const Point2i& filmDimension,
				const Point2i& raster,
				Ray* ray,
				CameraSample*) override;

			virtual Float
				generateRayDifferential(Sampler& sampler,
					const Point2i& filmDimension,
					const Point2i& raster,
					RayDifferential* ray, Float* weight) override {
				return 0.0;
			}
			Box<Camera> clone()const override {
				return makeBox<PerspectiveCamera>(*this);
			}
		private:
			void computeTransformMatrix();
		};
		MYK_IMPL(PerspectiveCamera, "Camera.PerspectiveCamera");
		MYK_REFL(PerspectiveCamera, (Camera), (lensRadius)(focalDistance)(fov));
	}
}


#endif