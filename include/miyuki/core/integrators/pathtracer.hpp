#ifndef MIYUKI_PATHTRACER_HPP
#define MIYUKI_PATHTRACER_HPP

#include <math/spectrum.h>
#include <core/scene.h>
#include <core/aovrecord.hpp>
#include <core/bsdfs/bsdf.h>
#include <core/medium/medium.h>
#include <core/phases/phase.h>
#include <utils/panic.h>
namespace Miyuki {
	namespace Core {
		class PathTracer {
		public:
			PathTracer(Scene& scene,
				SamplingContext& ctx,
				int minDepth,
				int maxDepth,
				bool useNEE)noexcept
				:scene(scene),
				ctx(ctx),
				minDepth(minDepth),
				maxDepth(maxDepth),
				depth(0),
				useNEE(useNEE),
				aov(ctx.cameraSample.pFilm), ray(ctx.primary) {
				option = ENoSampleOption;
				beta = Spectrum(1, 1, 1);
				ray.mediumStack = &mediumStack;
			}

			const AOVRecord& getAOV()const noexcept {
				return aov;
			}

			AOVRecord trace(Intersection* firstIsct)noexcept {
				computeLi(firstIsct);
				return getAOV();
			}

		protected:


			void lightSampling(bool handleMedia)noexcept {
				if (scene.getLights().empty())return;
				auto lightIdx = scene.getLightDistribution().sampleDiscrete(ctx.sampler->get1D());
				auto light = scene.getLights()[lightIdx];

				LightSamplingRecord record;
				VisibilityTester tester;
				record.u = ctx.sampler->get2D();
				light->sampleLi(isct, record, &tester);
				Float lightPdf = scene.getLightDistribution().pdfDiscrete(lightIdx) * record.pdf;
				Float scatteringPdf = 0;
				if (record.pdf > 0 && !record.Le.isBlack()) {
					// evaluate BSDF
					auto wo = isct.worldToLocal(isct.wo);
					auto wi = isct.worldToLocal(record.wi);
					Float cosTheta = Vec3f::absDot(isct.Ns, record.wi);
					Spectrum f;
					Spectrum Ld = record.Le / lightPdf;

					if (isct.isSurfaceScatteringEvent()) {
						scatteringPdf = isct.bsdf->evaluatePdf(wo, wi, option);
						f = isct.bsdf->evaluate(wo, wi, option) * cosTheta;
					}
					else if (isct.isVolumeScatteringEvent()) {
						scatteringPdf = isct.phase->evaluate(wo, wi);
						f = Spectrum(scatteringPdf);

					}
					else {
						//panic("isct must be either surface or volume");
						return;
					}

					// Visible and non-zero BSDF
					if (!f.isBlack()) {
						if (handleMedia) {
							Ld *= tester.Tr(scene, *ctx.sampler);
							/*	fmt::print("{}\n", (beta * f * Ld).max());*/
						}
						else {
							if (!tester.visible(scene)) {
								Ld = Spectrum(0.0);
							}
						}
						if (!Ld.isBlack()) {
							Float weight;
							if (!light->isDelta()) {
								// power heurisitcs
								weight = PowerHeuristics(lightPdf, scatteringPdf);
							}
							else {
								weight = 1.0f;
							}

							if (depth == 1) {
								if (handleMedia) {
									addLighting(BSDFLobe::ETransmission, beta * f * Ld * weight);
								}
								else {
									static const BSDFLobe lobes[] = { BSDFLobe::EDiffuse, BSDFLobe::EGlossy };
									Spectrum sum;
									for (const auto lobe : lobes) {
										auto fLobe = isct.bsdf->evaluate(wo, wi, option, lobe) * cosTheta;
										sum += fLobe;
										addLighting(lobe, beta * fLobe * Ld * weight);
									}
									addLighting(BSDFLobe::ESpecular, beta * (f - sum) * Ld * weight);
								}
							}
							else {
								addLighting(primaryLobe, beta * f * Ld * weight);
							}
						}
					}
				}
			}
			void phaseSamping(const MediumSample& m)noexcept {
				PhaseFunctionSample sample(isct, ctx.sampler);
				isct.phase->sample(sample);
				wi = sample.wi;
				ray = Ray(m.origin, wi, RayBias);
				ray.mediumStack = &mediumStack;
				prevScatteringPdf = isct.phase->evaluate(sample.wo, sample.wi);
				if (depth == 1) {
					primaryLobe = ETransmission;
				}
				//fmt::print("{} {} {}\n", beta.x, beta.y, beta.z);
			}

			void BSDFSampling() noexcept {

				sample = BSDFSample(isct, ctx.sampler, option);
				isct.bsdf->sample(sample);
				if (depth == 1) {
					primaryLobe = sample.lobe;
				}
				specular = sample.lobe & ESpecular;
				if (sample.pdf <= 0 || sample.f.isBlack()) {
					terminate();
					return;
				}
				wi = isct.localToWorld(sample.wi);
				ray = isct.spawnRay(wi);
				ray.mediumStack = &mediumStack;
				beta *= sample.f * Vec3f::absDot(isct.Ns, wi) / sample.pdf;
				prevScatteringPdf = sample.pdf;

			}
			void handleEnvMap()noexcept {
				auto light = scene.getEnvironmentLight();
				if (!light)return;
				auto L = light->L(ray);
				if (depth == 0)
					addLighting(EDiffuse, beta * L);
				else if (specular || !useNEE) {
					addLighting(primaryLobe, beta * L);
				}
				else {
					auto weight = computeMISWeight(light, sample.pdf, prevIsct);
					addLighting(primaryLobe, weight * beta * L);
				}
			}
			Float computeMISWeight(Light* light, Float scatterPdf, const Intersection& isct) noexcept {
				CHECK(useNEE);
				auto iter = scene.getLightPdfMap().find(light);
				Assert(iter != scene.getLightPdfMap().end());
				auto pdfLightSelect = iter->second;
				auto pdfLi = light->pdfLi(isct, wi);
				auto lightPdf = pdfLightSelect * pdfLi;
				auto weight = PowerHeuristics(scatterPdf, lightPdf);
				return weight;
			}
			void MIS() noexcept {
				auto light = isct.primitive->light();
				auto weight = computeMISWeight(light, prevScatteringPdf, prevIsct);
				addLighting(primaryLobe, beta * isct.Le(ray) * weight);
			}

			void nextIntersection()noexcept {
				prevIsct = isct;
				if (!intersect()) {
					handleEnvMap();
					terminate();
				}
				if (useNEE && isct.primitive->light() && !specular) {
					MIS();
				}
				else {
					addLighting(primaryLobe, beta * isct.Le(ray));
				}
			}

			void russianRoulette()noexcept {
				if (depth >= minDepth) {
					auto p = std::min(1.0f, beta.max());
					auto u = ctx.sampler->get1D();
					if (p == 1.0f || u < p) {
						beta /= p;
					}
					else {
						terminate();
					}
				}
			}

			void computeLi(Intersection* firstIsct)noexcept {
				if (firstIsct) {
					if (!firstIsct->hit()) {
						handleEnvMap();
						aov.normal = isct.wo;
						aov.albedo = aov.L();
						terminate();
					}
					else {
						isct = *firstIsct;
						if (!loadMaterial(isct)) {
							//
						}
						else {
							addLighting(EDiffuse, beta * firstIsct->Le(ray));
						}
					}


				}
				else {
					if (!intersect()) {
						handleEnvMap();
						aov.normal = isct.wo;
						aov.albedo = aov.L();
						terminate();
					}
					else {
						addLighting(EDiffuse, beta * isct.Le(ray));
					}

				}
				if (!continuable()) {
					return;
				}
				aov.normal = isct.Ns;
				ShadingPoint p(isct.textureUV);
				if (isct.primitive->material())
					aov.albedo = isct.primitive->material()->evalAlbedo(p);
				while (continuable()) {
					if (!isct.hit())break;
					MediumSample mediumSample(ray, ctx.sampler, ctx.arena);
					mediumSample.ray.tMax = isct.distance;
					auto Tr = sampleMedium(isct, mediumSample);
					//fmt::print("{}\n", Tr.min());
					beta *= Tr;
					if (beta.isBlack()) {
						terminate();
						break;
					}
					if (++depth > maxDepth)break;

					if (useNEE) {
						lightSampling(mediumSample.isValid());
						if (!continuable())break;
					}

					if (mediumSample.isValid()) {
						phaseSamping(mediumSample);
					}
					else if(isct.bsdf){
						BSDFSampling();
					}
					else {
						depth--;
						ray = isct.spawnRay(ray.d);
						ray.mediumStack = &mediumStack;
					}

					if (!continuable())break;

					nextIntersection();
					if (!continuable())break;

					russianRoulette();

				}
			}
			bool loadMaterial(Intersection& isct)noexcept {
				auto material = isct.primitive->material();
				if (!material)
					return false;
				BSDFCreationContext bsdfCtx(ShadingPoint(isct.textureUV), ctx.arena);
				auto impl = material->createBSDF(bsdfCtx);
				isct.bsdf = ARENA_ALLOC(*ctx.arena, BSDF)(impl, isct.Ng, isct.localFrame);
				return isct.bsdf != nullptr;
			}

			// Try to sample the medium
			// Returns (1,...) if no medium is presented
			Spectrum sampleMedium(Intersection& isct, MediumSample& sample)noexcept {
				auto medium = UpdateMediumStack(sample.ray, isct);
				if (!medium) {
					return Spectrum(1);
				}
				auto Tr = medium->sample(sample);
				isct.phase = sample.phase;
				//fmt::print("{}\n", Tr.max());
				return Tr;
			}

			bool intersect() {
				if (!scene.intersect(ray, &isct)) {
					return false;
				}
				loadMaterial(isct);
				return true;
			}

			void addLighting(BSDFLobe lobe, const Spectrum& lighting)noexcept {
				if (depth <= 1) {
					if (lobe & EDiffuse) {
						aov.aovs[EDiffuseDirect] += lighting;
					}
					else if (lobe & EGlossy) {
						aov.aovs[EGlossyDirect] += lighting;
					}
					else {
						aov.aovs[ESpecularAndTransmission] += lighting;
					}
				}
				else {
					if (lobe & EDiffuse) {
						aov.aovs[EDiffuseIndirect] += lighting;
					}
					else if (lobe & EGlossy) {
						aov.aovs[EGlossyIndirect] += lighting;
					}
					else {
						aov.aovs[ESpecularAndTransmission] += lighting;
					}
				}
			}
			bool continuable() noexcept {
				return _continue;
			}
			void terminate()noexcept {
				_continue = false;
			}
		private:
			BSDFSample sample;
			Vec3f wi;
			bool specular = false;
			BSDFLobe primaryLobe;
			Ray ray;
			Spectrum beta;
			BSDFSampleOption option;
			int depth;
			AOVRecord aov;
			Intersection isct, prevIsct;
			SamplingContext& ctx;
			Scene& scene;
			MediumStack mediumStack;
			Float prevScatteringPdf;
			int minDepth;
			int maxDepth;
			bool useNEE = false;
			bool _continue = true;
		};

	}
}

#endif  