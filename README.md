

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

<b>Bidirectional methods are disabled until I fathom the essence of path space measurement. (They won't converge to the reference result (path traced) at the moment.</b>

Although this is a CPU-only renderer, utilizing state-of-art rendering methods you can always get a nice image in around 5-10 minutes.

## How to build

Dependencies: Embree 3.0, boost filesystem(not required when on non-Windows).

CMake everything.

## Current Progress 

Available algorithms: path tracing, bidirectional path tracing (default),  and multiplexed MLT. BDPT is good at most scenes and converge to a nice results quickly. MLT is also a great solution which, for some scenes, delivers ultra fast convergence.

Available materials: lambertian, oren-nayar, microfacet glossy reflection, mirror

A GUI application is implemented solely for showing progressive rendering results.

##  TODO List

1. Volumetric Path tracing
2. ~~Ambient Occlusion~~
3. ~~Multiplexed Metropolis Light Transport~~
4. ~~Bidirectional Path Tracing with MIS~~ 
5. ~~Multiple Importance Sampling~~
6. Vertex Connection and Merging
7. ~~Importance sampling for light source~~
8. Mixed BSDF (partially)
9. Material system
10. Fourier BSDF
11. Environment mapping
12. Reversible Jump MLT ?

# Gallery (Random test images)

![](gallery/living_room.png)

![](gallery/sibenik.png)

![](gallery/vokselia_spawn_pr0.1.png)

![](gallery/ring.png)

![](gallery/sportsCar.png)

![](gallery/conference.png)

![](gallery/vokselia_spawn.png)

![](gallery/tough_box.png)


