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

		struct Camera : Component {
			MYK_INTERFACE(Camera);
			virtual Float generateRay(Sampler& sampler,
				const Point2i& raster,
				Ray* ray,
				CameraSample*) = 0;

			virtual Float
				generateRayDifferential(Sampler& sampler, 
					const Point2i& raster,
					RayDifferential* ray, Float* weight) = 0;

			virtual void preprocess() {  }	
			virtual Vec3f cameraToWorld(Vec3f w) const = 0;
			virtual Vec3f worldToCamera(Vec3f w) const = 0;
			virtual Arc<Film> createFilm() const = 0;
			virtual Box<Camera> scale(Float k) const = 0;
		};
		MYK_EXTENDS(Camera, (Component));

		struct PerspectiveCamera final: Camera {
			Vec3f viewpoint;
			// euler angle;
			Vec3f direction;
			Matrix4x4 rotationMatrix, invMatrix;
			Point2i dimension;
			Float lensRadius, focalDistance;
			Float fov;
			MYK_META(PerspectiveCamera);
			virtual void preprocess()override;

			virtual Vec3f cameraToWorld(Vec3f w) const override {
				w.w() = 1;
				w = rotationMatrix.mult(w);
				return w;
			}

			virtual Vec3f worldToCamera(Vec3f w) const override {
				w.w() = 1;
				w = invMatrix.mult(w);
				return w;
			}
			virtual Float generateRay(Sampler& sampler,
				const Point2i& raster,
				Ray* ray,
				CameraSample*) override;

			virtual Float
				generateRayDifferential(Sampler& sampler,
					const Point2i& raster,
					RayDifferential* ray, Float* weight) override {
				return 0.0;
			}
			Arc<Film> createFilm()const override;
			Box<Camera> scale(Float k)const override;
		private:
			void computeTransformMatrix();
		};
		MYK_IMPL(PerspectiveCamera, (Camera), "Camera.PerspectiveCamera")
		MYK_REFL(PerspectiveCamera, (viewpoint)(direction)(dimension)(lensRadius)(focalDistance)(fov));
	}
}


#endif