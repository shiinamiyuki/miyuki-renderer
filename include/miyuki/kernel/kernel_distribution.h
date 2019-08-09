#ifndef MIYUKI_KERNEL_DISTRIBUTION_H
#define MIYUKI_KERNEL_DISTRIBUTION_H

#include "kerneldef.h"

MYK_KERNEL_NS_BEGIN

typedef struct Distribution1D {
	float* cdf;
	int count;
}Distribution1D;

typedef struct Distribution2D {
	Distribution1D* conditional;
	Distribution1D marginal;
}Distribution2D;


// find i such that data[i] <= x < data[i+1]
MYK_KERNEL_FUNC 
int find_interval(float* data, int N, float x) {
	int lower = 0;
	int higher = N - 1;
	while (lower <= higher) {
		int mid = (lower + higher) / 2;
		if (mid + 1 >= N || (data[mid] <= x && x < data[mid + 1])) {
			return mid;
		}
		if (data[mid] < x) {
			lower = mid + 1;
		}
		else {
			higher = mid - 1;
		}
	}
	return (lower + higher) / 2;
}

MYK_KERNEL_FUNC
int distribution1d_sample_discrete(Distribution1D * dist, float u, float* pdf) {
	int i = (int)clamp(find_interval(ist, dist->count + 1, x), 0, dist->count - 1);
}


MYK_KERNEL_NS_END

#endif