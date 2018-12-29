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
	while (ctx.depth < scene->option.maxDepth) {
		intersect(ctx.ray, ctx.intersection);
		if (!ctx.intersection.hit()) {
			break;
		}
		ctx.ray.o += ctx.ray.d * ctx.intersection.distance;
		auto object = ctx.intersection.object;
		auto& material = object->getMaterial();
		vec3 wo;
		vec3 rad;
		Float prob;
		BxDFType type = material.sample(ctx.Xi, ctx.ray.d, ctx.intersection.normal, wo, rad, prob);
		if (prob < eps)
			break;
		if (type == BxDFType::none)break;
		
		if (type == BxDFType::emission) {
			if(ctx.sampleEmit)
				ctx.color += ctx.throughput * rad / prob;
			break;
		}
		ctx.throughput *= rad / prob;
		if (type == BxDFType::diffuse) {
			ctx.sampleEmit = false;
			auto direct = sampleLights(ctx);
			ctx.color += direct * ctx.throughput;
			ctx.throughput *= vec3::dotProduct(wo, ctx.intersection.normal)  * (2 * pi);
		}
		else {
			ctx.sampleEmit = true;
		}
		ctx.ray.d = wo;
		const Float P = max(ctx.throughput);
		if (ctx.depth > scene->option.rrStartDepth) {
			if (erand48(ctx.Xi) < P) {
				ctx.throughput /= P;
			}
			else {
				break;
			}
		}
		ctx.depth++;
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

void Miyuki::BDPT::intersect(const Ray & ray, Intersection &isct)
{
	isct.reset();
	scene->getAccel()->intersect(ray, isct);
}

void Miyuki::BDPT::traceLightPath(Seed*Xi, Path &path)
{
	Ray ray(vec3(0, 0, 0), vec3(0, 0, 0));
	vec3 radiance(0, 0, 0);
	vec3 refl(1, 1, 1);
	Intersection isct;
	{
		
		auto light = scene->lights[nrand48(Xi) % scene->lights.size()];
		ray.o = light->randomPointOnObject(Xi);
		auto n = light->getNormOfPoint(ray.o);
		ray.d = randomVectorInHemisphere(n, Xi);
		radiance = light->getMaterial().emittance
			* light->area() * 2 * pi * scene->lights.size();
		LightVertex v0;
		v0.normal = n;
		v0.wi = ray.d;
		v0.wo = vec3(0, 0, 0);
		v0.object = light;
		v0.type = BxDFType::emission;
		v0.radiance = radiance;
		v0.throughput = refl;
		v0.hitpoint = ray.o;
		isct.exclude = light;
		path.emplace_back(v0);
	}
	
	LightVertex v;
	
	for (int depth = 0; depth < scene->option.maxDepth; depth++) {
		intersect(ray, isct);
		if (!isct.hit())
			break;
		ray.o += ray.d * isct.distance;
		auto object = isct.object;
		auto& material = object->getMaterial();
		auto& Kd = material.diffuse;
		auto& Ks = material.specular;
		Float p1 = Kd.max(), p2 = Ks.max();
		Float total = p1 + p2;
		if (total < eps)break;

		vec3 wo;
		vec3 rad;
		Float prob;
		BxDFType type = material.sample(Xi, ray.d, isct.normal, wo, rad, prob);
		if (BxDFType::none == type) {
			break;
		}
		
		if (type == BxDFType::emission) {
			break;
		}
		v.normal = isct.normal;
		v.wi = wo;
		v.wo = ray.d;
		v.hitpoint = ray.o;
		v.object = object;
		v.type = type;
		
		if (type == BxDFType::diffuse) {
			refl *= std::max(Float(0), -vec3::dotProduct(ray.d, isct.normal));
		}
		
		refl *= rad / prob;
		v.radiance = radiance * refl;
		v.throughput = refl;
		
		
		ray.d = wo;
		path.emplace_back(v);
		if (type == BxDFType::diffuse) {
			refl *= 2 * pi;
		}
		Float P = refl.max();
		if (depth > scene->option.rrStartDepth) {
			if (erand48(Xi) < P) {
				refl /= P;
			}
			else {
				break;
			}
		}
		
	}

}

void Miyuki::BDPT::traceEyePath(RenderContext & ctx, Path &path)
{
	Ray& ray = ctx.ray;
	LightVertex v;
	while (ctx.depth < scene->option.maxDepth) {
		intersect(ctx.ray, ctx.intersection);
		if (!ctx.intersection.hit()) {
			break;
		}
		ctx.ray.o += ctx.ray.d * ctx.intersection.distance;
		auto object = ctx.intersection.object;
		auto& material = object->getMaterial();
		vec3 wo;
		vec3 rad;
		Float prob;
		BxDFType type = material.sample(ctx.Xi, ctx.ray.d, ctx.intersection.normal, wo, rad, prob);
		if (prob < eps)
			break;
		if (type == BxDFType::none)break;

		if (type == BxDFType::emission) {
			ctx.color = rad / prob;
			v.normal = ctx.intersection.normal;
			v.wi = ray.d;
			v.wo = wo;
			v.hitpoint = ray.o;
			v.object = object;
			v.type = type;
			v.radiance = ctx.color * ctx.throughput;
			v.throughput = ctx.throughput;
			path.emplace_back(v);
			break;
		}
		ctx.throughput *= rad / prob;
		v.normal = ctx.intersection.normal;
		v.wi = ray.d;
		v.wo = wo;
		v.hitpoint = ray.o;
		v.object = object;
		v.type = type;
		v.radiance = ctx.color * ctx.throughput;
		v.throughput = ctx.throughput;
		path.emplace_back(v);
		
		if (type == BxDFType::diffuse) {
			ctx.throughput *= std::max(Float(0), vec3::dotProduct(wo, ctx.intersection.normal)) * 2 * pi;
		}
		
		ctx.ray.d = wo;
		const Float P = max(ctx.throughput);
		if (ctx.depth > scene->option.rrStartDepth) {
			if (erand48(ctx.Xi) < P) {
				ctx.throughput /= P;
			}
			else {
				break;
			}
		}
		ctx.depth++;
	}
}

vec3 Miyuki::BDPT::connectPath(Path &eye, Path &light)
{
	vec3 radiance(0, 0, 0);
	Float weight = 0;
	for (int i = 0; i < eye.size(); i++) {
		auto& E = eye[i];
		if (E.type == BxDFType::emission) {
			Float w = 1.0 / (i + 1);
			weight += w;
			radiance += w * E.radiance;
			continue;
		}
		for (int j = 0; j < light.size(); j++) {
			auto& L = light[j];
			Float c = contribution(E, L);
			if (c > 0) {
				Float w = 1.0 / (i + j + 2);
				if (L.type == BxDFType::emission)
					radiance += E.throughput * c * L.radiance / (2 * pi) * w;
				else
					radiance += E.throughput * c * L.radiance * w;
				weight += w;
			}
		}
	}
	if (weight == 0)
		return vec3(0, 0, 0);
	return radiance / weight;
}

vec3 Miyuki::BDPT::trace(int x, int y)
{
	Path eye,light;
	auto ctx = scene->getRenderContext(x, y);
	traceLightPath(ctx.Xi, light);
	traceEyePath(ctx, eye);
	return connectPath(eye, light);
}

bool Miyuki::BDPT::visiblity(const LightVertex & v1, const LightVertex & v2)
{
	Ray ray(v1.hitpoint, (v2.hitpoint - v1.hitpoint).normalized());//from v1 to v2
	Intersection isct;
	isct.exclude = v1.object;
	if (vec3::dotProduct(v1.normal, ray.d) < 0 || -vec3::dotProduct(v2.normal, ray.d) < 0)
		return false;
	intersect(ray, isct);
	return isct.hit() && isct.object == v2.object;
}

Float Miyuki::BDPT::contribution(const LightVertex & v1, const LightVertex & v2)
{
	if ((v1.type == BxDFType::diffuse && v2.type == BxDFType::diffuse)
		|| (v1.type == BxDFType::diffuse && v2.type == BxDFType::emission)){
		if (!visiblity(v1, v2))return 0;
		return G(v1.normal,
			(v2.hitpoint - v1.hitpoint).normalized(),
			v2.normal,
			v1.hitpoint,
			v2.hitpoint);
	}
	return 0;
}

void Miyuki::BDPT::generateLightPath()
{
	int N = 1024;
	paths.resize(N);
	Seed Xi []= { 0,rand() };
	parallelFor(0u, (unsigned int)N, [&](unsigned int i) {
		paths[i].clear();
		traceLightPath(Xi, paths[i]);
	});
}

void Miyuki::BDPT::render(Scene *s)
{
	scene = s;
	logger = s->getLogger();
	vec3 dim = scene->getDimension();
	int w = dim.x();
	int h = dim.y();
	//generateLightPath();
	parallelFor(0, w, [&](unsigned int i) {
		int x = i;
		for (int y = 0; y < h; y++) {
			scene->getScreen()[x + scene->w * y] *= scene->sampleCount;
			scene->getScreen()[x + scene->w * y] += min(30 * vec3(1, 1, 1), trace(x, y));
			scene->getScreen()[x + scene->w * y] /= 1 + scene->sampleCount;
		}
	});
}
