//
// Created by Shiina Miyuki on 2019/1/22.
//

#include <math/distribution.h>
#include <math/distribution2d.h>

using namespace Miyuki;

Miyuki::Distribution1D::Distribution1D(const Float* data, uint32_t N) {
	cdfArray.resize(N + 1);
	cdfArray[0] = 0;
	for (int i = 0; i < N; i++) {
		cdfArray[i + 1] = cdfArray[i] + data[i];
	}
	funcInt = cdfArray[N];
	if (funcInt > 0) {
		for (int i = 0; i < N + 1; i++) {
			cdfArray[i] /= funcInt;
		}
	}
	else {
		for (int i = 1; i < N + 1; i++) {
			cdfArray[i] = Float(i) / N;
		}
	}
}

int Distribution1D::sampleDiscrete(Float x, Float* pdf) const {
	auto i = (int)clamp<size_t>(binarySearch(x), 0, cdfArray.size() - 2);
	if (pdf) {
		*pdf = this->pdf(i);
	}
	return i;
}

Float Distribution1D::sampleContinuous(Float u, Float* pdf, int* off) const {
	auto offset = (int)clamp<size_t>(binarySearch(u), 0, cdfArray.size() - 2);
	if (off) {
		*off = offset;
	}
	if (pdf) {
		*pdf = this->pdf(offset);
	}
	auto du = u - cdfArray[offset];
	du /= cdfArray[offset + 1] - cdfArray[offset];
	return (offset + du) / (cdfArray.size() - 1);
}

int Distribution1D::binarySearch(Float x) const {
	int lower = 0;
	int higher = (int)cdfArray.size() - 1;
	while (lower <= higher) {
		int mid = (lower + higher) / 2;
		if (mid + 1 >= cdfArray.size() || (cdfArray[mid] <= x && x < cdfArray[mid + 1])) {
			return mid;
		}
		if (cdfArray[mid] < x) {
			lower = mid + 1;
		}
		else {
			higher = mid - 1;
		}
	}
	return (lower + higher) / 2;
}

Float Distribution1D::cdf(Float x) const {
	int i = (int)clamp<Float>(x * cdfArray.size(), 0, cdfArray.size() - 1);
	return cdfArray[i];
}

Float Distribution1D::pdf(int x) const {
	return cdfArray[x + 1] - cdfArray[x];
}


Distribution2D::Distribution2D(const Float* data, size_t nu, size_t nv)const {
	for (auto v = 0; v < nv; v++) {
		pConditionalV.emplace_back(std::make_unique<Distribution1D>(&data[v * nu], nu));
	}
	std::vector<Float> m;
	for (auto v = 0; v < nv; v++) {
		m.emplace_back(pConditionalV[v]->funcInt);
	}
	pMarginal = std::make_unique<Distribution1D>(&m[0], nv);
}
