#include "kernel/trace.h"
#include "kernel/common/bvh.h"
#include "kernel/common/util.h"

float lightPdf(__global const Primitive * light, Vector p, float dist, Vector L, Vector N) {
	float area;
	if (light->type == TYPE_TRIANGLE) {
		area = light->area;
	}
	else if (light->type == TYPE_SPHERE) {
		float r = light->sphere.radius;
		area = pi * r * r;
	}
	return 1.0 / (area * -dot(N, L) / (dist * dist + 0.0001));
}
__global const Primitive * chooseLight(__global const RenderContext * ctx,int * i, Seed*Xi){
	__global const Primitive* objects = ctx->objects;
	int cnt = ctx->lightCount;
	__global const int *lights = ctx->lights;
	int idx = rand(Xi) * cnt;
	if (idx >= cnt) {
		idx %= cnt;
	}
	__global const Primitive * light = objects + lights[idx];
	*i = idx;
	return light;
}

Vector sampleLights(
	__global const RenderContext * ctx,
	int id,
	Vector pt, 
	Vector hitNorm,
	Seed*Xi,
	float * pdf) {
	Vector color = vec3(0, 0, 0);
	float factor = 1.0f;
	int cnt = ctx->lightCount;
	__global const Primitive* objects = ctx->objects;
	__global const int *lights = ctx->lights;
	int idx;
	__global const Primitive * light = chooseLight(ctx, &idx,Xi);
	Vector norm;
	Vector p;
	float area;
	float u = rand(Xi);
	float v = rand(Xi);
	if (light->type == TYPE_TRIANGLE) {
		norm = light->triangle.norm;
		Vector e1 = light->triangle.edge1;
		Vector e2 = light->triangle.edge2;
		norm = light->triangle.norm;
		area = light->area;
			
		p = u * e1 + (1 - u) * v * e2 + light->triangle.vertex0;
	}
	else if(light->type == TYPE_SPHERE) {
		float r = light->sphere.radius;
		area = pi * r * r;
		p = randomVectorInSphereU1U2(u,v);
		norm = p;
		p *= r;
		p += light->sphere.center;
	}
	Ray shadow;
	shadow.ro = pt;
	shadow.rd = normalize(p - pt);
	Intersection isct;
	initIntersection(&isct);
	findIntersection(&shadow, ctx, &isct);
	float d = length(p - pt);
	float PDF = lightPdf(light, p, d, shadow.rd, norm);
	*pdf = PDF;
	if (hit(&isct)
		&& isct.id == lights[idx]
		) {		
		color += ctx->materials[light->material].ambient 
			/ fabs(PDF)
			* max(0.0f, dot(shadow.rd, hitNorm));
	}
	return color;
}
Vector sampleLightsGlossy(
	__global const RenderContext * ctx,
	int id,
	Vector pt,
	Vector hitNorm,
	Seed*Xi,
	float * pdf,
	Vector * L) {
	Vector color = vec3(0, 0, 0);
	float factor = 1.0f;
	int cnt = ctx->lightCount;
	__global const Primitive* objects = ctx->objects;
	__global const int *lights = ctx->lights;
	int idx;
	__global const Primitive * light = chooseLight(ctx, &idx, Xi);
	Vector norm;
	Vector p;
	float area;
	float u = rand(Xi);
	float v = rand(Xi);
	if (light->type == TYPE_TRIANGLE) {
		norm = light->triangle.norm;
		Vector e1 = light->triangle.edge1;
		Vector e2 = light->triangle.edge2;
		norm = light->triangle.norm;
		area = light->area;

		p = u * e1 + (1 - u) * v * e2 + light->triangle.vertex0;
	}
	else if (light->type == TYPE_SPHERE) {
		float r = light->sphere.radius;
		area = pi * r * r;
		p = randomVectorInSphereU1U2(u, v);
		norm = p;
		p *= r;
		p += light->sphere.center;
	}
	Ray shadow;
	shadow.ro = pt;
	shadow.rd = normalize(p - pt);
	Intersection isct;
	initIntersection(&isct);
	findIntersection(&shadow, ctx, &isct);
	float d = length(p - pt);
	float PDF = lightPdf(light, p, d, shadow.rd, norm);
	*pdf = PDF;
	if (hit(&isct)
		&& isct.id == lights[idx]
		) {
		color += ctx->materials[light->material].ambient
			/ fabs(PDF);
	}
	*L = shadow.rd;
	return color;
}

Vector radiance(__global const RenderContext * ctx, Ray ray, Seed *Xi) {
	Vector color = vec3(0, 0, 0);
	Vector refl = vec3(1, 1, 1);
	Intersection isct;
	initIntersection(&isct);
	float wL = 1;
	__global const Material * materials = ctx->materials;
	for (int depth = 0; depth < 5; depth++) {
		findIntersection(&ray, ctx, &isct);
		if (!hit(&isct)) {
			break;
		}
		ray.ro += isct.distance * ray.rd;
		__global const Primitive * object = ctx->objects + isct.id;
		float glossiness = materials[object->material].roughness;
		Vector Ka = materials[object->material].ambient;
		Vector Kd = materials[object->material].diffuse;
		Vector Ks = materials[object->material].specular;
		float prob;
		float p1 = vmax(Ka);
		float p2 = vmax(Kd);
		float p3 = vmax(Ks);
		float total = p1 + p2 + p3;
		if (total < eps) {
			break;
		}
		float p = rand(Xi) * total;
		float pdf;
		if (p < p1) {
			prob = p1 / total;
			refl /= prob;
			color += Ka * refl * wL;
			break;
		}
		else if (p < p1 + p2) {
			prob = p2 / total;
			Vector BRDF = Kd / pi;
			refl *= BRDF / prob;
			Vector direct = sampleLights(ctx, isct.id, ray.ro, isct.norm, Xi, &pdf);
			ray.rd = cosineWeightedSampling(&isct.norm, Xi);
			float pdfIndirect = dot(ray.rd, isct.norm) / pi;
			pdf *= pdf;
			pdfIndirect *= pdfIndirect;
			float w = pdfIndirect / (pdf + pdfIndirect);
			direct *= pdf / (pdf + pdfIndirect);
			color += direct * refl;			
			refl *= pi;
			wL = pdfIndirect / (pdf + pdfIndirect);
		}
		else {
			wL = 1.0;
			Vector norm = isct.norm;
			if (glossiness > eps) {
				norm = GGXImportanceSampling(glossiness, &isct.norm, Xi);
			}
			float Tr = ctx->materials[object->material].Tr;
			float Ni = ctx->materials[object->material].Ni;
			bool doRefl = false;
			if (rand(Xi) < Tr) {
				prob = p3 / total * Tr;
				float n1, n2;
				if (dot(ray.rd, norm) < 0) {
					n1 = 1;
					n2 = Ni;
				}
				else {
					n1 = Ni;
					n2 = 1;
					norm *= -1.0;
				}
				float cosI = fabs(dot(ray.rd, norm));
				float n12 = n1 / n2;
				float s2 = n12 * n12*(1 - cosI * cosI);
				float root = 1 - s2;
				float T,R;
				refl *= Ks / prob;
				if (root >= 0) {
					float cosT = sqrt(root);
					float Rpara = (n1 * cosI - n2 * cosT) / (n1 * cosI + n2 * cosT);
					float Rorth = (n2 * cosI - n1 * cosT) / (n2 * cosI + n1 * cosT);
					R = (Rpara * Rpara + Rorth * Rorth) / 2;
				}
				else {
					R = 1;
				}
				T = 1 - R;
				if(rand(Xi) < R){ 
					doRefl = true;
					refl *= Ks / prob / R;
				}else{
					refl *= Ks / prob / T;
				}
			}
			else {
				prob = p3 / total * (1 - Tr);
				refl *= Ks / prob;
				doRefl = true;
			}
			if (doRefl) {
				if (glossiness > eps) {
					Vector L;
					Vector direct = sampleLightsGlossy(ctx, isct.id, ray.ro,isct.norm, Xi, &pdf, &L);
					Vector microfacet = normalize((L - ray.rd) / 2); 
					float dist = GGXDistribution(microfacet, isct.norm, glossiness);
					float pdfIndirect = GGXDistribution(norm, isct.norm, glossiness);
					pdf *= pdf;
					pdfIndirect *= pdfIndirect;
					color += dist * direct * refl * pdf / (pdf + pdfIndirect);
					wL = pdfIndirect / (pdf + pdfIndirect);
				}
				ray.rd = reflect(norm, ray.rd);
			}

		}
		float P = vmax(refl);
		if (rand(Xi) < P) {
			refl /= P;
		}
		else {
			break;
		}
	}
	return color;
}


Vector integrator(__global const RenderContext * ctx, Ray ray, Seed *Xi) {
	return min(vec3(1,1,1) * 3,radiance(ctx, ray, Xi));
}