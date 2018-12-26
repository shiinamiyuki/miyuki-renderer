#include "PathTracer.h"
#include "Scene.h"
#include "vec.h"

using namespace Miyuki;
vec3 Miyuki::PathTracer::sampleLights(RenderContext & ctx)
{
	return sampleLights(ctx.ray, ctx.intersection, ctx.Xi);
}

vec3 Miyuki::PathTracer::sampleLights(const Ray & ray, Intersection & isct, Seed * Xi)
{
	vec3 color(0, 0, 0);
	//for (auto light : scene->lights) {
	auto light = scene->lights[nrand48(Xi) % scene->lights.size()];
	if (isct.object == light) {
		return  light->getMaterial().emittance;
	}
	auto p = light->randomPointOnObject(Xi);
	auto area = light->area();
	auto shadow = p - ray.o;
	auto distSqr = shadow.lengthSquared();
	shadow.normalize();
	Intersection vis;
	vis.exclude = isct.object;
	Ray shadowRay(ray.o + eps * shadow, shadow);
	intersect(shadowRay, vis);
	if (vis.hit() && vis.object == light) {
		color += area / (distSqr + eps)
			* std::max(Float(0.0), -(vec3::dotProduct(shadow, vis.normal)))
			* light->getMaterial().emittance
			* std::max(Float(0.0), (vec3::dotProduct(shadow, isct.normal)));
	}
	//}
	return color * scene->lights.size();
}

void Miyuki::PathTracer::intersect(const Ray & ray, Intersection &intersection)
{
	intersection.reset();
	//bvh->intersect(ray, intersection);
	scene->accel->intersect(ray, intersection);
}

void Miyuki::PathTracer::prepare()
{
	camera = scene->camera;

}

vec3 Miyuki::PathTracer::trace(int x0, int y0)
{

	RenderContext ctx = scene->getRenderContext(x0, y0);
	while (ctx.depth < 5) {
		intersect(ctx.ray, ctx.intersection);
		if (!ctx.intersection.hit()) {
			break;
		}
		ctx.ray.o += ctx.ray.d * ctx.intersection.distance;
		if (!ctx.intersection.object->getMaterial().render(this,ctx))
			break;
	}
	return ctx.color;
}

vec3 Miyuki::PathTracer::raycast(int x0, int y0)
{
	RenderContext ctx = scene->getRenderContext(x0, y0);
	int m = scene->option.maxDepth;
	while (ctx.depth < 1) {
		intersect(ctx.ray, ctx.intersection);
		if (!ctx.intersection.hit()) {
			ctx.color += scene->worldColor; break;
		}
		ctx.ray.o += ctx.ray.d * ctx.intersection.distance;
		return ctx.intersection.object->getMaterial().diffuse
			* std::max(Float(0.2),vec3::dotProduct(ctx.intersection.normal,vec3(0.2,1,0).normalized()))
			+ ctx.intersection.object->getMaterial().emittance;
		ctx.ray.o += ctx.ray.d * eps * 4;
		if (ctx.color.max() > 1)
			break;
	}
	return min(ctx.color, 10*vec3(3, 3, 3));
}

void Miyuki::PathTracer::render(Scene *s)
{
	scene = s;
	logger = s->getLogger();
	vec3 dim = scene->getDimension();
	int w = dim.x();
	int h = dim.y();
	parallelFor(0, w, [&](unsigned int i) {
		int x = i;
		for (int y = 0; y < h; y++) {
			scene->getScreen()[x + scene->w * y] *= scene->sampleCount;
			scene->getScreen()[x + scene->w * y] += min(3 * vec3(1, 1, 1), trace(x, y));
			scene->getScreen()[x + scene->w * y] /= 1 + scene->sampleCount;
		}
	});
}
