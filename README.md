

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

A GUI application is implemented solely for showing progressive rendering results.

##  TODO List

1. Volumetric Path tracing with NEE
2. ~~Ambient Occlusion~~
3. Stochastic Progressive Photon Mapping
4. Path Space MLT
5. ~~Primary Sample Space Metropolis Light Transport (Unidirectional)~~
6. Primary Sample Space Metropolis Light Transport (Bidirectional)
7. ~~Multiplexed Metropolis Light Transport~~
8. ~~Bidirectional Path Tracing with MIS~~ 
9. ~~Multiple Importance Sampling~~
10. Vertex Connection and Merging
11. ~~Importance sampling for light source~~
12. Mixed BSDF (partially)
13. Material system
14. Fourier BSDF
15. Environment mapping

# Gallery (Random test images)

![](gallery/vokselia_spawn_pr0.1.png)

![](gallery/sportsCar.png)

![](gallery/conference.png)

![](gallery/vokselia_spawn.png)

![](gallery/tough_box.png)


