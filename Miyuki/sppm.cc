#include "sppm.h"
#include "Scene.h"
#include "Intersection.h"
#include "Primitive.h"

using namespace Miyuki;

vec3 Miyuki::SPPM::sampleLights(RenderContext & ctx)
{
	return sampleLights(ctx.ray, ctx.intersection, ctx.Xi);
}

vec3 Miyuki::SPPM::sampleLights(const Ray & ray, Intersection & isct, Seed * Xi)
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
	Ray shadowRay(ray.o, shadow);
	intersect(shadowRay, vis);
	if (vis.hit() && vis.object == light) {
		color += area / (distSqr + 0.001)
			* std::max(Float(0.0), -(vec3::dotProduct(shadow, vis.normal)))
			* light->getMaterial().emittance
			//* (1 - isct.object->getMaterial().reflectance(shadow * -1,isct.normal))
			* std::max(Float(0.0), (vec3::dotProduct(shadow, isct.normal)))
			*  light->getMaterial().emissionStrength;
	}
	//}
	return color * scene->lights.size();
}
void Miyuki::SPPM::init(int w, int h)
{
	image.clear();
	image.resize(w * h);
	for (auto& i : image) {
		i.radius = scene->option.sppm.initialRadius * 0.01 * scene->getAccel()->dim();
		i.alpha = scene->option.sppm.alpha;
	}
}

void Miyuki::SPPM::computeTotalPower()
{
	powerIntergral.clear();
	power = 0;
	for (auto i : scene->lights) {
		power += i->getMaterial().emittance.max();
		powerIntergral.emplace_back(power);
	}
}

Primitive * Miyuki::SPPM::searchLight(Float powerInt)
{
	unsigned int low = 0;
	unsigned int upper = powerIntergral.size();
	while (low < upper) {
		int mid = (low + upper) / 2;
		if (powerIntergral[mid] < powerInt
			&& (mid == powerIntergral.size() - 1 || powerInt < powerIntergral[mid + 1])) {
			return scene->lights[mid];
		}
		else {
			if (powerIntergral[mid] < powerInt) {
				low = mid;
			}
			else {
				upper = mid;
			}
		}
	}
	return scene->lights[low];
}

void Miyuki::SPPM::emitPhoton(Float total, vec3 & flux, Ray & ray, Seed * Xi)
{
	// TODO: shoot photons of equal flux
	//Float p = erand48(Xi) * power;
	//auto light = searchLight(p);
	auto light = scene->lights[nrand48(Xi) % scene->lights.size()];
	ray.o = light->randomPointOnObject(Xi);
	auto n = light->getNormOfPoint(ray.o);
	ray.d = randomVectorInHemisphere(n, Xi);
	// ???
	flux = light->getMaterial().emittance * light->getMaterial().emissionStrength
		* light->area() * scene->lights.size()
		* pi * 2; //why not pi
}
void Miyuki::SPPM::tracePhoton(Float N, Seed * Xi)
{
	vec3 flux;
	Ray ray(vec3(0, 0, 0), vec3(0, 0, 0));
	emitPhoton(N, flux, ray, Xi); 
	Intersection isct;
	Float F = flux.max();
	bool direct = true;
	for (int depth = 0; depth < scene->option.maxDepth; depth++) {
		intersect(ray, isct);
		if (!isct.hit())
			break;
		ray.o += isct.distance * ray.d;
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
		if (type == BxDFType::emission)break;
		
		if (type == BxDFType::diffuse) {
			if(!direct
				|| scene->option.sppm.direct == Scene::Option::SPPMpara::DirectLighting::sppmDirect)
				pushPhoton(Photon(ray.o, ray.d, flux));
			flux *= pi;// std::max(Float(0), -vec3::dotProduct(ray.d, isct.normal)) * 2 * pi;
		}
		direct = false;
		flux *= rad / prob;
		ray.d = wo;
		Float P = flux.max();
		if (depth > scene->option.rrStartDepth) {
			if (erand48(Xi) * F < P) {
				flux /= P / F;
			}
			else {
				break;
			}
		}
	}
}

void Miyuki::SPPM::intersect(const Ray & ray, Intersection & isct)
{
	isct.reset();
	scene->getAccel()->intersect(ray, isct);
}

void Miyuki::SPPM::photonPass(int N)
{
	Ne += N;
	photonMap.clear();
	photons.clear();
	int r = rand();
	Seed Xi[] = { 0, r };
	parallelFor(0u, N, [&](unsigned int i) {
		
		tracePhoton(N, Xi);
	});
	//logger->log("photons = {}\n", photons.size());
}

void Miyuki::SPPM::cameraPass()
{
	parallelFor(0u, (unsigned int)scene->w, [&](unsigned int i) {
		for (int j = 0; j < scene->h;j++) {
			
			auto ctx = scene->getRenderContext(i, j);
			Ray& ray = ctx.ray;
			auto& r = image[i + scene->w * j];
			bool diffuse = false;
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
					ctx.color += ctx.throughput * rad / prob;
					break;
				}
				ctx.throughput *= rad / prob;
				if (type == BxDFType::diffuse) {
					vec3 lighting = r.estimateRadiance(
						this,
						ctx.throughput,
						ctx.ray.o,
						ctx.intersection.normal);
					ctx.color += lighting;
					if (scene->option.sppm.direct == Scene::Option::SPPMpara::DirectLighting::sppmDirect) {
						diffuse = true;
					}else {
						ctx.color += ctx.throughput * sampleLights(ctx);
					}
					break;
				}
				ctx.ray.d = wo;
				const Float P = max(ctx.throughput);
				if (erand48(ctx.Xi) < P) {
					ctx.throughput /= P;
				}
				else {
					break;
				}
				ctx.depth++;
			}
			r.nonDiffuse *= scene->sampleCount;
			if (!diffuse) {
				r.nonDiffuse += min(4 * vec3(1,1,1),ctx.color);
			}
			r.nonDiffuse /= 1 + scene->sampleCount;

			scene->getScreen()[i + scene->w * j] = min(3 * vec3(1, 1, 1), r.nonDiffuse + r.radiance);
			
		}
	});
}

void Miyuki::SPPM::buildPhotonMap()
{
	//logger->log("Building photon map\nPhoton Count={}\n", photons.size());
	try {
		photonMap.construct(photons, 0, photons.size(), 0);
		decltype(photons) dummy;
		std::swap(photons, dummy);
	}
	catch (std::exception&e) {
		logger->log("exception caught: {}\nCannot build photon map\n", e.what());
	}
}

void Miyuki::SPPM::pushPhoton(Photon &&p)
{
	std::lock_guard<std::mutex> lock(photonMutex);
	photons.push_back(p);
}

vec3 Miyuki::SPPM::Region::estimateRadiance(SPPM * sppm, const vec3& refl, const vec3 & p0, const vec3 & norm)
{
	Photon p;
	p.pos = p0;
	decltype(photonMap)::PointVec nn;
	//Float r = sppm->photonMap.knn(p, radius, 100, nn) + eps;
	sppm->photonMap.findWithin(p, radius, nn);
	vec3 rad = vec3(0, 0, 0);
	for (int i = 0; i < nn.size(); i++) {
		rad += nn[i].flux
			 *std::max(Float(0), -vec3::dotProduct(nn[i].dir, norm));
	}
	rad *= refl;
	auto R = radius;
	auto M = nn.size();
	auto Phi = rad;
	if (N > 0) {
		radius = R * sqrt((N + alpha * M) / (N + M)); //R_{i+1}
		
		flux = (flux + Phi) * (radius / R) * (radius / R);
		N = N + alpha * nn.size();
	}
	else {
		N = nn.size();
		flux = Phi;
	}
	Float frac = (sppm->Ne * pi * radius * radius);
	radiance = flux / frac;
	constexpr Float maxR = 4;
	if (radiance.max() > maxR) {
		flux *= maxR / radiance.max();
		radiance *= maxR / radiance.max();
		
	}
	return radiance;
}

SPPM::SPPM()
{
	scene = nullptr;
}

void Miyuki::SPPM::render(Scene *s)
{
	scene = s;
	logger = s->getLogger();
	if (scene->sampleCount<=0) {
		Ne = 0;
		logger->log("sppm lookup radius = {}\n", scene->option.sppm.initialRadius * 0.01 * scene->accel->dim());
		init(scene->getDimension().x(), scene->getDimension().y());
		computeTotalPower();
	}
	photonPass(scene->option.sppm.numPhoton);
	buildPhotonMap();
	cameraPass();	
}


SPPM::~SPPM()
{
}
