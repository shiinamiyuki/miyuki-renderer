#ifndef MIYUKI_PATHTRACER_HPP
#define MIYUKI_PATHTRACER_HPP

#include <math/spectrum.h>
#include <core/scene.h>
#include <core/aovrecord.hpp>
#include <core/bsdfs/bsdf.h>

namespace Miyuki {
	namespace Core {
		class PathTracer {
		public:
			PathTracer(Scene& scene,
				SamplingContext& ctx,
				int minDepth,
				int maxDepth,
				bool useNEE)
				:scene(scene),
				ctx(ctx),
				minDepth(minDepth),
				maxDepth(maxDepth),
				depth(0),
				useNEE(useNEE),
				aov(ctx.cameraSample.pFilm), ray(ctx.primary) {
				option = ENoSampleOption;
				beta = Spectrum(1,1,1);
			}

			const AOVRecord& getAOV()const {
				return aov;
			}

			AOVRecord trace() {
				computeLi();
				return getAOV();
			}

		protected:
			void handleEnvMap() {	
				auto light = scene.getEnvironmentLight();
				if (!light)return;
				auto L = light->L(ray);
				if (depth == 0)
					addLighting(EDiffuse, beta * L);
				else
					addLighting(primaryLobe, beta * L);
			}

			void lightSampling() {
				if (scene.getLights().empty())return;
				auto lightIdx = scene.getLightDistribution().sampleInt(ctx.sampler->get1D());
				auto light = scene.getLights()[lightIdx].get();

				LightSamplingRecord record;
				VisibilityTester tester;
				record.u = ctx.sampler->get2D();
				light->sampleLi(isct, record, &tester);
				Float lightPdf = scene.getLightDistribution().pdf(lightIdx) * record.pdf;
				if (record.pdf > 0 && !record.Le.isBlack()) {
					// evaluate BSDF
					auto wo = isct.worldToLocal(isct.wo);
					auto wi = isct.worldToLocal(record.wi);
					Float cosTheta = Vec3f::absDot(isct.Ns, record.wi);
					Spectrum f = isct.bsdf->evaluate(wo, wi, option) * cosTheta;

					// Visible and non-zero BSDF
					if (!f.isBlack() && tester.visible(scene)) {
						Spectrum Ld = record.Le / lightPdf;
						auto scatteringPdf = isct.bsdf->evaluatePdf(wo, wi, option);
						Float weight;
						if (!light->isDelta()) {
							// power heurisitcs
							weight = PowerHeuristics(lightPdf, scatteringPdf);
						}
						else {
							weight = 1.0f;
						}
						if (depth == 1) {
							static const BSDFLobe lobes[] = { BSDFLobe::EDiffuse, BSDFLobe::EGlossy };
							Spectrum sum;
							for (const auto lobe : lobes) {
								auto fLobe = isct.bsdf->evaluate(wo, wi, option, lobe) * cosTheta;
								sum += fLobe;
								addLighting(lobe, beta * fLobe * Ld * weight);
							}
							addLighting(BSDFLobe::ESpecular, beta * (f - sum) * Ld * weight);
						}
						else {
							addLighting(primaryLobe, beta * f * Ld * weight);
						}
					}
				}
			}

			void BSDFSampling() {
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
				wi = isct.localToWord(sample.wi);
				ray = isct.spawnRay(wi);
				beta *= sample.f * Vec3f::absDot(isct.Ns, wi) / sample.pdf;
			}

			void MIS() {
				auto light = isct.primitive->light();
				auto iter = scene.getLightPdfMap().find(light);
				Assert(iter != scene.getLightPdfMap().end());
				auto pdfLightSelect = iter->second;
				auto pdfLi = light->pdfLi(prevIsct, wi);
				auto lightPdf = pdfLightSelect * pdfLi;
				auto weight = PowerHeuristics(sample.pdf, lightPdf);
				addLighting(primaryLobe, beta * isct.Le(ray) * weight);
			}

			void nextIntersection() {
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

			void russianRoulette() {
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

			void computeLi() {
				if (!intersect()) {
					handleEnvMap();
					terminate();
				}
				addLighting(EDiffuse, beta * isct.Le(ray));
				while (continuable()) {
					if (!isct.hit())break;
					if (++depth > maxDepth)break;
					CHECK(isct.bsdf);

					if (useNEE) {
						lightSampling();
						if (!continuable())break;
					}

					BSDFSampling();
					if (!continuable())break;

					nextIntersection(); 
					if (!continuable())break;

					russianRoulette(); 

				}
			}

			bool intersect() {
				if (!scene.intersect(ray, &isct)) {
					return false;
				}
				material = isct.primitive->material();
				if (!material)
					return false;
				BSDFCreationContext bsdfCtx(ShadingPoint(isct.textureUV), ctx.arena);
				auto impl = material->createBSDF(bsdfCtx);
				isct.bsdf = ARENA_ALLOC(*ctx.arena, BSDF)(impl, isct.Ng, isct.localFrame);
				return isct.bsdf != nullptr;
			}

			void addLighting(BSDFLobe lobe, const Spectrum& lighting) {
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
			bool continuable() {
				return _continue;
			}
			void terminate() {
				_continue = false;
			}
		private:
			BSDFSample sample;
			Vec3f wi;
			bool specular = false;
			BSDFLobe primaryLobe;
			Ray ray;
			Spectrum beta;
			Material* material = nullptr;
			BSDFSampleOption option;
			int depth;
			AOVRecord aov;
			Intersection isct, prevIsct;
			SamplingContext& ctx;
			Scene& scene;
			int minDepth;
			int maxDepth;
			bool useNEE = false;
			bool _continue = true;
		};

	}
}

#endif  