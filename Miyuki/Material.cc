#include "Material.h"
#include "PathTracer.h"
#include "Primitive.h"
using namespace Miyuki;

bool Miyuki::Material::render(PathTracer* tracer,RenderContext &ctx)const
{
	// ray.o is at intersection point
	const auto& Ka = emittance;
	const auto& Kd = diffuse;
	const auto& Ks = specular;
	//const auto& Ka = ambient;
	auto& n = ctx.intersection.normal;

	const Float p1 = max(Ka);
	const Float p2 = max(Kd);
	const Float p3 = max(Ks);
	const Float total = p1 + p2 + p3;
	Float prob = 1;
	if (total < eps) {
		return false;
	}
	const Float dice = erand48(ctx.Xi) * total;
	if (dice < p1) {
		prob = p1 / total;
		if (ctx.directLighting && !ctx.sampleEmit)return false;
		ctx.color += Ka * ctx.throughput / prob;
		return false;
	}
	else if (dice < p1 + p2) {
		const auto BRDF = Kd / pi;
		const Float p = 1 / (2 * pi);
		prob = p2 / total;
		ctx.sampleEmit = false;
		ctx.ray.d = randomVectorInHemisphere(ctx.intersection.normal, ctx.Xi);
		ctx.throughput *= BRDF / prob;
		if (ctx.directLighting) {
			auto direct = tracer->sampleLights(ctx);
			ctx.color += direct* ctx.throughput;
		}
		ctx.throughput *= 1.0 / p * (vec3::dotProduct(ctx.ray.d, n));
	}
	else {
		ctx.sampleEmit = true;
		if (erand48(ctx.Xi) < Tr) {
			prob = p3 / total * Tr;
			Float n1, n2;
			if (vec3::dotProduct(ctx.ray.d, n) < 0) {
				n1 = 1;
				n2 = Ni;
			}
			else {
				n1 = Ni;
				n2 = 1;
				n *= -1.0;
			}
			Float c = -vec3::dotProduct(ctx.ray.d, n);
			Float n12 = n1 / n2;
			Float s2 = n12 * n12*(1 - c * c);
			Float root = 1 - s2;
			if (root >= 0) {
				ctx.ray.d = n12 * ctx.ray.d + (n12 * c - sqrt(root))*n;
				ctx.ray.d.normalize();
				ctx.throughput *= Ks / prob;
			}
			else {
				ctx.ray.d = ctx.ray.d - 2 * (vec3::dotProduct(ctx.ray.d, n)) * n;
				ctx.throughput *= Ks / prob;
			}
		}
		else {
			prob = p3 / total * (1 - Tr);
			ctx.ray.d = ctx.ray.d - 2 * (vec3::dotProduct(ctx.ray.d, n)) * n;
			ctx.throughput *= Ks / prob;
		}
	}

	// Russian Roulette
	const Float P = max(ctx.throughput);
	if (P < 1) {
		if (erand48(ctx.Xi) < P) {
			ctx.throughput /= P;
		}
		else {
			return false;
		}
	}
	++ctx.depth;
	return true;
}

vec3 Miyuki::refract(const vec3 & dir, const vec3 & norm, const Float Ni, Float & prob)
{
	return vec3();
}
