//
// Created by Shiina Miyuki on 2019/2/9.
//
#include "integrator.h"
#include "../core/scatteringevent.h"
using namespace Miyuki;
ScatteringEvent
Integrator::makeScatteringEvent(const Ray &ray, IntersectionInfo *info, Sampler *sampler) {
    return ScatteringEvent( info, sampler);
}

