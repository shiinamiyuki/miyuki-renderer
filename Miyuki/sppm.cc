#include "sppm.h"
#include "Scene.h"
#include "Intersection.h"
#include "Primitive.h"
using namespace Miyuki;


void Miyuki::SPPM::init(int w, int h)
{
	image.clear();
	image.resize(w * h);
	for (auto i : image) {
		i.radius = scene->option.sppm.initialRadius;
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
	ray.d = randomVectorInHemisphere(light->getNormOfPoint(ray.o), Xi);
	ray.o += eps * ray.d;
	flux = light->getMaterial().emittance 
		* light->area() * 2 * pi * scene->lights.size();
}

void Miyuki::SPPM::tracePhoton(Float N, Seed * Xi)
{
	vec3 flux;
	Ray ray(vec3(0, 0, 0), vec3(0, 0, 0));
	emitPhoton(N, flux, ray, Xi);
	Intersection isct;
	Float F = flux.max();
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
			photons.emplace_back(Photon(ray.o, ray.d, flux));
			flux *= std::max(Float(0), -vec3::dotProduct(ray.d, isct.normal)) * 2 * pi;
		}
		flux *= rad / prob;
		ray.d = wo;
		Float P = flux.max();
		if (erand48(Xi) * F < P) {
			flux /= P /F;
		}
		else {
			break;
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
	Seed *Xi = scene->Xi;
	For(0u, N, [&](unsigned int i) {
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
					diffuse = true;
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
				r.nonDiffuse += ctx.color;
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

vec3 Miyuki::SPPM::Region::estimateRadiance(SPPM * sppm, const vec3& refl, const vec3 & p0, const vec3 & norm)
{
	Photon p;
	p.pos = p0;
	std::vector<Photon> nn;
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
	radius = R * sqrt((N + alpha * M) / (N + M)); //R_{i+1}
	auto Phi = rad;
	flux = (flux + Phi) * (radius / R) * (radius / R);
	N = N + alpha * nn.size();
	Float frac = (sppm->Ne * pi * radius * radius);
	radiance =  flux / frac ;
	constexpr Float maxR = 3;
	if (radiance.max() > maxR) {
		radiance *= maxR / radiance.max();
		flux *= maxR / radiance.max();
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
