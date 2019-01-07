#include "kernel/trace.h"
#include "kernel/common/bvh.h"
#include "kernel/common/util.h"

#define PATH_NULL		(0)
#define PATH_AMBIENT	(1)
#define PATH_DIFFUSE	(2)
#define PATH_SPECULAR	(3)

typedef struct LightVertex {
	Vector hitPoint;
	Vector norm;
	Vector refl;
	Vector dir;
	int hitObject;
	int path;
	float prob;
}LightVertex;

#define MAX_LIGHT_PATH 5
#define MAX_EYE_PATH 5

void determinePath(
	__global const Primitive * object,
	Ray *ray,
	Intersection*isct,
	LightVertex* vertex,
	Seed *Xi) {
	ray->ro += isct->distance * ray->rd;
	vertex->hitObject = isct->id;
	vertex->hitPoint = ray->ro;
	vertex->norm = isct->norm;
	Vector Ka = object->material.ambient;
	Vector Kd = object->material.diffuse;
	Vector Ks = object->material.specular;
	float p1 = vmax(Ka);
	float p2 = vmax(Kd);
	float p3 = vmax(Ks);
	float total = p1 + p2 + p3;
	if (total < eps) {
		vertex->path = PATH_NULL;
		return;
	}
	float p = rand(Xi) * total;
	if (p < p1) {
		vertex->prob = p1 / total;
		vertex->refl = Ka;
		vertex->path = PATH_AMBIENT;
	}
	else if (p < p1 + p2) {
		vertex->prob = p2 / total;
		Vector BRDF = Kd / pi;
		vertex->refl = BRDF;
		ray->rd = cosineWeightedSampling(&isct->norm, Xi);
		vertex->path = PATH_DIFFUSE;
	}
	else {
		vertex->refl = Ks;
		vertex->path = PATH_SPECULAR;
		float Tr = object->material.Tr;
		float Ni = object->material.Ni;
		if (rand(Xi) < Tr) {
			vertex->prob = p3 / total * Tr;
			float n1, n2;
			if (dot(ray->rd, isct->norm) < 0) {
				n1 = 1;
				n2 = Ni;
			}
			else {
				n1 = Ni;
				n2 = 1;
				isct->norm *= -1.0;
			}
			float c = fabs(dot(ray->rd, isct->norm));
			float n12 = n1 / n2;
			float s2 = n12 * n12*(1 - c * c);
			float root = 1 - s2;
			if (root >= 0) {
				ray->rd = n12 * ray->rd + (n12 * c - sqrt(root))*isct->norm;
			}
			else {
				ray->rd = reflect(isct->norm, ray->rd);
			}
		}
		else {
			vertex->prob = p3 / total * (1 - Tr);
			ray->rd = reflect(isct->norm, ray->rd);
		}
	}
	vertex->dir = ray->rd;
}

void traceLightPath(
	__global RenderContext * ctx,
	LightVertex * lightPath,
	int * numOfVertex,
	Seed*Xi) {
#if MAX_LIGHT_PATH < 1
	* numOfVertex = 0;
	return;
#else
	int lightIdx = rand(Xi) * ctx->lightCount;
	if (lightIdx >= ctx->lightCount)
		lightIdx %= ctx->lightCount;
	Vector R = vec3(0,0,0);
	lightPath[0].hitObject = ctx->lights[lightIdx];
	{
		Vector norm;
		Vector p;
		float area;
		__global const Primitive * light = &ctx->objects[lightPath[0].hitObject];
		lightPath[0].refl = light->material.ambient;
		if (light->type == TYPE_TRIANGLE) {
			norm = light->triangle.norm;
			Vector e1 = light->triangle.edge1;
			Vector e2 = light->triangle.edge2;
			norm = cross(e1, e2);
			area = length(norm) / 2;
			norm = light->triangle.norm;
			float u = rand(Xi);
			float v = rand(Xi);
			p = u * e1 + (1 - u) * v * e2 + light->triangle.vertex0;
		}
		else if (light->type == TYPE_SPHERE) {
			float r = light->sphere.radius;
			area = pi * r * r;
			p = randomVectorInSphere(Xi);
			norm = p;
			p *= r;
			p += light->sphere.center;
		}
		lightPath[0].hitPoint = p;
		lightPath[0].norm = norm;
		lightPath[0].prob = 1.0f / area;
		lightPath[0].path = PATH_NULL;
	}
	Ray ray;
	ray.ro = lightPath[0].hitPoint;
	ray.rd = cosineWeightedSampling(&lightPath[0].norm, Xi);
	ray.ro += 4 * eps * ray.rd;
	int i = 1;
	for (; i < MAX_LIGHT_PATH; i++) {
		Intersection isct;
		findIntersection(&ray, ctx, &isct);
		if (!hit(&isct))
			break;
		__global const Primitive * object = ctx->objects + isct.id;
		determinePath(object, &ray, &isct, &lightPath[i], Xi);
		if (lightPath[i].path == PATH_NULL)
			break;
		float P = vmax(lightPath[i].refl);
		/*if (rand(Xi) < P) {
			lightPath[i].refl /= P;
		}
		else {
			break;
		}*/
		ray.ro += 4 * eps * ray.rd;
	}
	*numOfVertex = i;
#endif
}

void traceEyePath(
	Ray r,
	__global RenderContext * ctx,
	LightVertex * eyePath,
	int * numOfVertex,
	Seed*Xi) {
	Ray ray = r;
	int i = 0;
	for (; i < MAX_EYE_PATH; i++) {
		Intersection isct;
		findIntersection(&ray, ctx, &isct);
		if (!hit(&isct))
			break;
		__global const Primitive * object = ctx->objects + isct.id;
		determinePath(object, &ray, &isct, &eyePath[i], Xi);
		if (eyePath[i].path == PATH_NULL)
			break;
		if (eyePath[i].path == PATH_AMBIENT) { i++; break; }
		float P = vmax(eyePath[i].refl);
		/*if (rand(Xi) < P) {
			eyePath[i].refl /= P;
		}
		else {
			break;
		}*/
		ray.ro += 4 * eps * ray.rd;
	}
	*numOfVertex = i;
}
// we will discard the `eye path = 0` case
// assuming Visibility is true
Vector connectPath(
	__global RenderContext * ctx,
	LightVertex * eyePath,
	int E,
	LightVertex* lightPath,
	int L) {
	Vector color = vec3(0, 0, 0);
	Vector refl = vec3(1, 1, 1);
	for (int i = 0; i < E; i++) {
		if (i > 1) {
			refl *= dot(eyePath[i - 1].norm, eyePath[i - 1].dir);
		}
		int path = eyePath[i].path;
		if (path == PATH_DIFFUSE) {
			refl *= eyePath[i].refl * pi;
		}
		else if (path == PATH_AMBIENT) {
			color += eyePath[i].refl * refl;
			return color;
		}
		else if (path == PATH_SPECULAR) {
			refl *= eyePath[i].refl;
		}
		if (i != E - 1)
			refl /= eyePath[i].prob;
	}
	// now we want to compute the geometry term :D
	Vector dir = lightPath[L - 1].hitPoint - eyePath[E - 1].hitPoint;
	float distSqr = dot(dir, dir);
	dir /= sqrt(distSqr);
	float cosX1_X2 = max(0.0f, dot(eyePath[E - 1].norm, dir));
	float cosX2_X3 = max(0.0f, -dot(dir, lightPath[L - 1].norm));
	float G = cosX1_X2 * cosX2_X3 / (eps + distSqr);

	for (int i = L - 1; i >= 1; i--) {
		refl /= lightPath[i].prob;
		int path = lightPath[i].path;
		if (path == PATH_AMBIENT) {
			color += lightPath[i].refl * refl;
		}
		else if (path == PATH_DIFFUSE) {
			refl *= lightPath[i].refl * pi;
		}
		else if (path == PATH_SPECULAR) {
			refl *= lightPath[i].refl;
		}
	}
	color += lightPath[0].refl * refl / lightPath[0].prob;// *2 * pi;
	return color * G;
}

bool visible(__global RenderContext * ctx, Vector from, Vector to, int toId) {
	Vector dir = normalize(to - from);
	Ray ray;
	ray.ro = from;
	ray.rd = dir;
	ray.ro += 4 * eps * ray.rd;
	Intersection isct;
	findIntersection(&ray, ctx, &isct);
	return hit(&isct) && isct.id == toId;
}

Vector BDPT(__global RenderContext * ctx, Ray ray, Seed *Xi) {
	LightVertex lightPath[MAX_LIGHT_PATH];
	int lightPathCount;
	LightVertex eyePath[MAX_EYE_PATH];
	int eyePathCount;
	traceEyePath(ray, ctx, eyePath, &eyePathCount, Xi);
	traceLightPath(ctx, lightPath, &lightPathCount, Xi);
	Vector color = vec3(0, 0, 0);
	float weight = 0;
	for (int i = 0; i < eyePathCount; i++) {
		for (int j = 0; j < lightPathCount; j++) {
			Vector from, to;
			from = eyePath[i].hitPoint;
			to = lightPath[j].hitPoint;
			int p1 = eyePath[i].path;
			int p2 = lightPath[j].path;
			int toId = lightPath[j].hitObject;
			if (p1 == PATH_AMBIENT || (p1 != PATH_SPECULAR && p2 != PATH_SPECULAR
				&& visible(ctx, from, to, toId))) {
				float w = 1.0/(i+1) + 1.0/(j+1);
				color += connectPath(ctx, eyePath, i + 1, lightPath, j + 1) * w;
				weight += w;;
			}
		}
	}
	color = weight > 0 ? (color / weight) : color;
	return color;
}
Vector integrator(__global RenderContext * ctx, Ray ray, Seed *Xi) {
	return BDPT(ctx, ray, Xi);
}