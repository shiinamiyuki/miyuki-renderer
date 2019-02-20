

# Miyuki Renderer(WIP)

```
        _             _    _     __                _
  /\/\ (_)_   _ _   _| | _(_)   /__\ ___ _ __   __| | ___ _ __ ___ _ __
 /    \| | | | | | | | |/ / |  / \/// _ \ '_ \ / _` |/ _ \ '__/ _ \ '__|
/ /\/\ \ | |_| | |_| |   <| | / _  \  __/ | | | (_| |  __/ | |  __/ |
\/    \/_|\__, |\__,_|_|\_\_| \/ \_/\___|_| |_|\__,_|\___|_|  \___|_|
          |___/
```

Goal: a high performance pbrt-style renderer. Using Embree for accelerated ray-primitive intersection.

## How to build

Dependencies: Embree 3.0, boost filesystem(not required when on non-Windows).

CMake everything.

## Current Progress 

Available algorithms: path tracing, bidirectional path tracing (default), PSSMLT and multiplexed MLT. The MLTs aren't working well though.

Available materials: lambertian, oren-nayar, microfacet glossy reflection, mirror

##  TODO List

1. Volumetric Path tracing with NEE
2. ~~Ambient Occlusion~~
3. Stochastic Progressive Photon Mapping
4. ~~Primary Sample Space Metropolis Light Transport (Unidirectional)~~
5. Primary Sample Space Metropolis Light Transport (Bidirectional)
6. ~~Multiplexed Metropolis Light Transport~~
7. ~~Bidirectional Path Tracing with MIS~~ 
8. ~~Multiple Importance Sampling~~
9. Vertex Connection and Merging
10. ~~Importance sampling for light source~~
11. Mixed BSDF (partially)
12. Material system
13. Fourier BSDF
14. Environment mapping

# Gallery (Random test images)

![](gallery/vokselia_spawn_pr0.1.png)

![](gallery/conference.png)

![](gallery/vokselia_spawn.png)

![](gallery/conference.png)

![](gallery/tough_box.png)



# About the Miyuki_old

That's my another attempt in creating a physically based renderer. It has many algorithms implemented but it's not that efficient comparing to Embree and GPU based renderer, despite the fact that I have used SIMD intrinsic to speed up ray-primitive.

There are reasons why this new attempt is not GPU based. One of them is that writing OpenCL (my laptop doesn't have NV GPU) involves lots of nasty stuff and hard-to-debug code, not to mention the fact the some rendering algorithms requires tens to thousands of Ms of additional memory.<br/>
Edit: Tried to write OpenCL again, the compiler is giving me tons of segfault. Nevertheless, BVH+SAH on GPU is really fun.