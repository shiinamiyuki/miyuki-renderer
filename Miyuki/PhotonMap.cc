#include "PhotonMap.h"
#include "Scene.h"
#include "Primitive.h"
using namespace Miyuki;
void Miyuki::PhotonMapRenderer::computeTotalPower()
{
	powerIntergral.clear();
	power = 0;
	for (auto i : scene->lights) {
		power += i->getMaterial().emittance.max();
		powerIntergral.emplace_back(power);
	}
}

Primitive * Miyuki::PhotonMapRenderer::searchLight(Float powerInt)
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

void Miyuki::PhotonMapRenderer::emitPhoton(Float total,vec3& flux, Ray&ray,Seed*Xi)
{
	Float p = erand48(Xi) * power;
	auto light = searchLight(p);
	ray.o = light->randomPointOnObject(Xi);
	ray.d = randomVectorInHemisphere(light->getNormOfPoint(ray.o),Xi);
	flux = light->getMaterial().emittance / total
		* light->area() * 2 * pi;
}

void Miyuki::PhotonMapRenderer::tracePhotonIdx(Float N,unsigned int idx, Seed*Xi)
{
	vec3 flux;
	Ray ray(vec3(0, 0, 0), vec3(0, 0, 0));
	emitPhoton(N,flux, ray, Xi);
	Intersection isct;
	bool first = true;
	for (int depth = 0; depth < 5; depth++) {
		intersect(ray, isct);
		if (!isct.hit())
			break;
		ray.o += isct.distance * ray.d;
		auto object = isct.object;
		auto& material = object->getMaterial();
		auto& Kd = material.diffuse;
		auto& Ks = material.specular;
		/*auto total = vec3::dotProduct(Kd + Ks, vec3(1, 1, 1));
		if (total < eps)break;
		auto Pr = (Kd + Ks).max();
		auto Pd = vec3::dotProduct(Kd, vec3(1, 1, 1)) / total * Pr;
		auto Ps = Pr - Pd;
		auto z = erand48(Xi);
		if (z < Pd) {
			flux /= Pd;
			photons[idx].emplace_back(Photon(ray.o, ray.d, flux));
			ray.d = randomVectorInHemisphere(isct.normal, Xi);
		}
		else if (z < Ps + Pd) {
			flux /= Ps;
			ray.d = ray.d - 2 * (vec3::dotProduct(ray.d, isct.normal)) * isct.normal;
		}
		else { //absorption
			break;
		}*/
		Float p1 = Kd.max(), p2 = Ks.max();
		Float total = p1 + p2;
		if (total < eps)break;
		Float p = total * erand48(Xi);
		if (p < p1) {
			flux /= p1 / total;
			if (erand48(Xi) > p1) {
				break;
			}
			if (!first)
				photons.emplace_back(Photon(ray.o, ray.d, flux));
			else
				first = false;
			ray.d = randomVectorInHemisphere(isct.normal, Xi);
			flux *= Kd * std::max(Float(0), vec3::dotProduct(ray.d, isct.normal)) * 2;
		}
		else {
			first = false;
			flux /= p2 / total;
			if (erand48(Xi) > p2) {
				break;
			}
			auto Tr = material.Tr;
			auto Ni = material.Ni;
			auto& n = isct.normal;
			Float prob;
			if (erand48(Xi) < Tr) {
				prob = Tr;
				Float n1, n2;
				if (vec3::dotProduct(ray.d, n) < 0) {
					n1 = 1;
					n2 = Ni;
				}
				else {
					n1 = Ni;
					n2 = 1;
					n *= -1.0;
				}
				Float c = -vec3::dotProduct(ray.d, n);
				Float n12 = n1 / n2;
				Float s2 = n12 * n12*(1 - c * c);
				Float root = 1 - s2;
				if (root >= 0) {
					ray.d = n12 * ray.d + (n12 * c - sqrt(root))*n;
					ray.d.normalize();
					flux *= Ks / prob;
				}
				else {
					ray.d = ray.d - 2 * (vec3::dotProduct(ray.d, n)) * n;
					flux *= Ks / prob;
				}
			}
			else {
				prob =(1 - Tr);
				ray.d = ray.d - 2 * (vec3::dotProduct(ray.d, n)) * n;
				flux *= Ks / prob;
			}
		}
		Float P = flux.max();
		if (P < 1) {
			if (erand48(Xi) < P) {
				flux /= P;
			}
			else {
				break;
			}
		}
		ray.o += 4 * eps * ray.d;
	}
}

void Miyuki::PhotonMapRenderer::reset()
{
	photons.clear();
	photonMap.clear();
}

void Miyuki::PhotonMapRenderer::tracePhoton(unsigned long long N)
{
	logger->log("Tracing {} photons\n", N);
	photons.resize(N); 
	Seed Xi[2] = { 0,rand() };
	For(0u, N, [&](unsigned int i) {
		
		tracePhotonIdx(N,i, Xi);
	});
	for (auto&i : photons) {
		//i.flux /= N;
	}
}

void Miyuki::PhotonMapRenderer::intersect(const Ray & ray, Intersection &isct)
{
	isct.reset();
	scene->accel->intersect(ray, isct);
}

void Miyuki::PhotonMapRenderer::buildPhotonMap()
{
	logger->log("Building photon map\nPhoton Count={}\n",photons.size());
	try {
		photonMap.construct(photons, 0, photons.size(), 0);
		parameter.usePhoton = std::min((double)parameter.maxPhoton, sqrt(photons.size()));
		decltype(photons) dummy;
		std::swap(photons, dummy);
	//	auto box = scene->accel->dim();
		parameter.radius = parameter.initalRadius;// (box.max - box.min).max() * parameter.initalRadius;
	}
	catch (std::exception&e) {
		logger->log("exception caught: {}\nCannot build photon map\n",e.what());
	}
}

vec3 Miyuki::PhotonMapRenderer::estimateRadiance(const vec3 & p0, const vec3&norm)
{
	const Float k = 3;
	Photon p;
	p.pos = p0;
	std::vector<Photon> nn;
	Float r = photonMap.knn(p, parameter.radius, parameter.usePhoton, nn);
	vec3 radiance = vec3(0, 0, 0);
	for (int i = 0; i < nn.size(); i++) {
		radiance += nn[i].flux
			* std::max(Float(0), -vec3::dotProduct(nn[i].dir, norm))
			* (1 - (p0 - nn[i].pos).length() / (k*r));
	}
	return radiance / (pi * r * r + eps) / (1 - 2.0 / k / 3.0);
}

vec3 Miyuki::PhotonMapRenderer::traceEyePath(int x0, int y0)
{
	int idx = x0 + y0 * scene->w;
	Seed * Xi = &scene->Xi[x0 + y0 * scene->w];
	Float x = (2 * (Float)x0 / scene->w - 1)* static_cast<Float>(scene->w) / scene->h;
	Float y = 2 * (1 - (Float)y0 / scene->h) - 1;
	vec3 ro = scene->camera.pos;
	Float dx = 2.0 / scene->h, dy = 2.0 / scene->h;
	vec3 jitter = vec3(dx*erand48(Xi), dy*erand48(Xi), 0);
	Float z = 2.0 / tan(scene->camera.aov / 2);
	vec3 rd = vec3(x, y, 0) + jitter - vec3(0, 0, -z);
	rd.normalize();
	rd = rotate(rd, vec3(1, 0, 0), scene->camera.dir.y());
	rd = rotate(rd, vec3(0, 1, 0), scene->camera.dir.x());
	vec3 refl(1, 1, 1);
	Ray ray(ro, rd);
	Intersection isct;
	for (int depth = 0; depth < 5; depth++) {
		intersect(ray, isct);
		if (!isct.hit())
			break;
		ray.o += isct.distance * ray.d;
		auto object = isct.object;
		auto& material = object->getMaterial();
		auto& Ka = material.emittance;
		auto& Kd = material.diffuse;
		auto& Ks = material.specular;
		
		const auto p1 = Ka.max();
		const auto p2 = Kd.max();
		const auto p3 = Ks.max();
		auto total = p1 + p2 + p3;
		auto d = erand48(Xi) * total;
		if (d < p1) {
			return refl * Ka / (p1/total);
		}
		else if (d < p1 + p2) {
			if (!indirectCache[idx].v[3]) {
				indirectCache[idx] = estimateRadiance(ray.o, isct.normal) / (p2 / total);
				indirectCache[idx].v[3] = true;
			}
		//	return refl *Kd * estimateRadiance(ray.o, isct.normal) / (p2 / total);
			return refl * 
				Kd*(1.0 / pi*scene->tracer.sampleLights(ray, isct, Xi)
					 +indirectCache[idx]);
		}
		else {
			auto Tr = material.Tr;
			auto Ni = material.Ni;
			auto& n = isct.normal;
			Float prob;
			if (erand48(Xi) < Tr) {
				prob = p3 / total * Tr;
				Float n1, n2;
				if (vec3::dotProduct(ray.d, n) < 0) {
					n1 = 1;
					n2 = Ni;
				}
				else {
					n1 = Ni;
					n2 = 1;
					n *= -1.0;
				}
				Float c = -vec3::dotProduct(ray.d, n);
				Float n12 = n1 / n2;
				Float s2 = n12 * n12*(1 - c * c);
				Float root = 1 - s2;
				if (root >= 0) {
					ray.d = n12 * ray.d + (n12 * c - sqrt(root))*n;
					ray.d.normalize();
					refl *= Ks / prob;
				}
				else {
					ray.d = ray.d - 2 * (vec3::dotProduct(ray.d, n)) * n;
					refl *= Ks / prob;
				}
			}
			else {
				prob = p3 / total * (1 - Tr);
				ray.d = ray.d - 2 * (vec3::dotProduct(ray.d, n)) * n;
				refl *= Ks / prob;
			}
			//refl *= Ks / (p3 / total);
			//ray.d = ray.d - 2 * (vec3::dotProduct(ray.d, isct.normal)) * isct.normal;
		}
		ray.o += 4 * eps * ray.d;
	}
	return vec3(0,0,0);
}

void Miyuki::PhotonMapRenderer::trace()
{
	parallelFor(0, scene->w, [&](unsigned int i) {
		int x = i;
		for (int y = 0; y < scene->h; y++) {
			scene->screen[x + scene->w * y] *= scene->sampleCount;
			scene->screen[x + scene->w * y] += min(3*vec3(1,1,1),traceEyePath(x, y));
			scene->screen[x + scene->w * y] /= 1 + scene->sampleCount;
		}
	});
}

void Miyuki::PhotonMapRenderer::prepare()
{

}

void Miyuki::PhotonMapRenderer::render()
{
	if (scene->sampleCount <= 0) {
		indirectCache.resize(scene->w*scene->h);
		for (auto&i : indirectCache) {
			i.v[3] = false;
		}
		if (photonMap.nodes.empty()) {
			reset();
			computeTotalPower();
			tracePhoton(2000000);
			buildPhotonMap();
			logger->log("rendering\n");
		}
	//	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	trace();
}

void Miyuki::PhotonMapRenderer::render(Scene * s)
{
	scene = s;
	logger = scene->getLogger();
	render();
}
