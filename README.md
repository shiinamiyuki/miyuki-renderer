# Miyuki Renderer(WIP)

Goal: a high performance pbrt-style renderer.

## Current Progress

We only have basic path tracing and ambient occlusion.

##  Will have following features

1. Accelerated ray-primitive intersection using Embree
2. Volumetric Path tracing with NEE
3. Ambient Occlusion
4. Stochastic Progressive Photon Mapping
5. Primary Sample Space Metropolis Light Transport
6. Bidirectional Path Tracing
7. Multiple Importance Sampling
8. Vertex Connection and Merging

# About the Miyuki_old

That's my another attempt in creating a physically based renderer. It has many algorithms implemented but it's not that efficient comparing to Embree and GPU based renderer, despite the fact that I have used SIMD intrinsic to speed up ray-primitive.

There are reasons why this new attempt is not GPU based. One of them is that writing OpenCL (my laptop doesn't have NV GPU) involves lots of nasty stuff and hard-to-debug code, not to mention the fact the some rendering algorithms requires tens to thousands of Ms of additional memory.