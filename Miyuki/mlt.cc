#include "mlt.h"
#include "Scene.h"
using namespace Miyuki;
Float Miyuki::SamplerBase::sample(Seed * Xi)
{
	return erand48(Xi);
}

Float Miyuki::MLTSampler::primarySample(int i, Seed*Xi)
{
	if (samples[i].lastModifyIter < curIter) {
		if (largeStep) {
			stack.emplace_back(std::make_pair(i,samples[i]));
			samples[i].lastModifyIter = curIter;
			samples[i].value = erand48(Xi);
		}
		else {
			if (samples[i].lastModifyIter < lastLargeStepIter) {
				samples[i].lastModifyIter = lastLargeStepIter;
				samples[i].value = erand48(Xi);
			}
			while (samples[i].lastModifyIter < curIter - 1) {
				samples[i].value = mutate(Xi, samples[i].value, 1);	
				samples[i].lastModifyIter++;
			}
			stack.emplace_back(std::make_pair(i, samples[i]));
			samples[i].value = mutate(Xi, samples[i].value, 1);
			samples[i].lastModifyIter++;
		}
	}
	return samples[i].value;
}

Miyuki::MLTSampler::MLTSampler() {
	largeStepProb = 0.5;
	idx = 0; curIter = lastLargeStepIter = 0;
	seed[0] = rand();
	seed[1] = rand();
	seed[2] = rand();
	samples.reserve(32);
}

Float Miyuki::MLTSampler::sample(Seed * Xi)
{
	if (idx >= samples.size()) {
		while (idx >= samples.size()) {
			samples.emplace_back(PrimarySample());
			samples.back().value = erand48(Xi);
			primarySample(samples.size() - 1, Xi);
		}
	}
	else {
		primarySample(idx, Xi);
	}
	return samples[idx++].value;
}




void Miyuki::MLTSampler::accept()
{
	if (largeStep)
		lastLargeStepIter = curIter;
	stack.clear();
}

void Miyuki::MLTSampler::reject()
{
	while (!stack.empty() ){
		auto p = stack.back();
		stack.pop_back();
		samples[p.first] = p.second;
	}
}

void Miyuki::MLTSampler::newSeed()
{
	idx = 0;
	for (auto &i : samples) {
		i.value = erand48(seed);
	}stack.clear();
}

void Miyuki::MLTSampler::finishBoostrap()
{
	for (auto &i : samples) {
		i.lastModifyIter = -1;
	}
	lastLargeStepIter = 0;
	stack.clear();
}

void Miyuki::MLTSampler::assignSeed(const std::vector<Float>&x)
{
	samples.clear();
	for (auto i : x) {
		samples.push_back(PrimarySample());
		samples.back().value = i;
		samples.back().lastModifyIter = 0;
	}
	stack.clear();
}

Float Miyuki::MLTSampler::mutate(Seed*Xi,Float x,Float n)
{
	constexpr Float sigma = 0.1;
	
	constexpr float s1 = 1.0 / 1024.0,s2 = 1.0/64.0;
	for (int i = 0; i < n; i++) {
		float dv = s2 * exp(-log(s2 / s1)*erand48(Xi));
		if (erand48(Xi) < 0.5) {
			x += dv;
			if (x > 1)x -= 1;
		}
		else {
			x -= dv;
			if (x < 0)x += 1;
		}
	}
	return x;
}



void Miyuki::MLTSampler::mutate()
{//mutation

}


void Miyuki::MLTSampler::update(Scene*, const vec3& p, const vec3 & Fy)
{

	auto I = luminance(Fy);
	Float a = std::max(Float(0),std::min(Float(1), I / (oldI)));
	if (isnan(a))
		a = 1;
	auto Wnew = std::max<double>(0,(a + largeStep) / (I / b + largeStepProb));
	auto Wold = std::max<double>(0,(1.0 - a) / (oldI / b + largeStepProb));
	if (isnan(Wnew))
		Wnew = 0;
	if (isnan(Wold))
		Wold = 0;
	newSample.weight = Wnew;
	newSample.pos = p;
	newSample.contrib = Fy;
	oldSample.weight += Wold;
	if (erand48(seed) < a) { //accepted
		contribSample = oldSample;
		oldSample = newSample;
		oldI = I;
		accept();
		curIter++;
	}
	else {
		contribSample = newSample;
		reject();
	}
	
}



vec3 Miyuki::MLT::trace(MLTSampler &sp, int _x, int _y, bool boot)
{
	auto _ctx = scene->getRenderContext(_x, _y);
	sp.start();
	int x = Sampler::toInt(sp.sample(_ctx.Xi), scene->w);
	int y = Sampler::toInt(sp.sample(_ctx.Xi), scene->h);
	RenderContext ctx = scene->getRenderContext(x, y);
	ctx.sampler.s = &sp;
	ctx.Xi = _ctx.Xi;
	auto Fy = min(vec3(1, 1, 1) * 4 , PathTracer::trace(ctx));
	sp.update(scene, vec3(x, y, 0), Fy);
	return Fy;
}
class RandSampler : public SamplerBase {
public:
	std::vector<float> path;

	float sample(Seed*Xi)override {
		auto x = erand48(Xi);
		path.emplace_back(x);
		return x;
	}
};
int binarySearch(const std::vector<Float> &cdf, Float x) {
	int lower = 0;
	int upper = cdf.size() - 1;
	while (lower <= upper) {
		int mid = (lower + upper) / 2;
		if (mid == 0) {
			return mid;
		}
		if (cdf[mid - 1] <= x && x < cdf[mid]) {
			return mid;
		}
		if (cdf[mid] < x) {
			lower = mid + 1;
		}
		else {
			upper = mid - 1;
		}
	}
	return (lower + upper) / 2;
}
void Miyuki::MLT::bootstrap()
{
	sampler.clear();
	sampler.resize(scene->w * scene->h);
	b = 0;
	constexpr int boostrapN = 100000;
	std::vector<Float> I,sumI; //sumI[i] = sum(I,0, i)
	std::vector<RandSampler> seeds;
	std::vector<vec3>pos;
	std::vector<vec3> Le;
	seeds.resize(boostrapN);
	std::random_device rd;
	std::uniform_real_distribution<Float> dist;
	std::uniform_int_distribution<int> idist;
	Seed Xi[3] = { idist(rd),  idist(rd) , idist(rd) };
	for (int i = 0; i < boostrapN; i++) {
		auto &s = seeds[i];
		int x = Sampler::toInt(s.sample(Xi), scene->w);
		int y = Sampler::toInt(s.sample(Xi), scene->h);
		auto ctx = scene->getRenderContext(x, y);
		ctx.sampler.s = &s;
		auto L = min(vec3(1,1,1)*4,PathTracer::trace(ctx));
		I.emplace_back(luminance(L));
		Le.emplace_back(L);
		pos.emplace_back(vec3(x, y, 0));
	}
	sumI.emplace_back(0);
	for (auto i : I) {
		sumI.emplace_back(sumI.back() + i);
	}
	b = sumI.back() / boostrapN;
	for (auto & i : sampler) {
		int idx = binarySearch(sumI, sumI.back() * dist(rd));
		if (idx < 0)idx = 0;
		if (idx >= seeds.size())idx = seeds.size() - 1;
		i.assignSeed(seeds[idx - 1].path);
		i.oldSample.pos = pos[idx];
		i.oldSample.contrib = Le[idx];
		i.oldI = I[idx];
		i.oldSample.weight = 0;
		i.b = b; 
		i.seed[2] = idist(rd);
		i.seed[3] = idist(rd);
	}
	
}

void Miyuki::MLT::render(Scene *s)
{
	scene = s;
	logger = s->getLogger();
	if (scene->getSampleCount() == 0) {
		bootstrap();
	};
	parallelFor(0, scene->w, [&](unsigned int i) {
		int x = i;
		for (int y = 0; y < scene->h; y++) {
			trace(sampler[x + scene->w * y], x, y);
		}
	});
	For(0, scene->w * scene->h, [&](unsigned int i) {
		scene->getScreen()[i] *=  scene->getSampleCount();
	});
	For(0, scene->w * scene->h, [&](unsigned int i) {
		auto & s = sampler[i];
		scene->getScreen()[s.contribSample.pos.x() + scene->w *s.contribSample.pos.y()]
			+= s.contribSample.contrib * s.contribSample.weight;
	});
	For(0, scene->w * scene->h, [&](unsigned int i) {
		scene->getScreen()[i] /= 1 + scene->getSampleCount();
	});
}

MLT::MLT() :PathTracer(nullptr)
{
}


MLT::~MLT()
{
}
