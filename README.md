

# Miyuki Renderer(WIP)

```
        _             _    _     __                _
  /\/\ (_)_   _ _   _| | _(_)   /__\ ___ _ __   __| | ___ _ __ ___ _ __
 /    \| | | | | | | | |/ / |  / \/// _ \ '_ \ / _` |/ _ \ '__/ _ \ '__|
/ /\/\ \ | |_| | |_| |   <| | / _  \  __/ | | | (_| |  __/ | |  __/ |
\/    \/_|\__, |\__,_|_|\_\_| \/ \_/\___|_| |_|\__,_|\___|_|  \___|_|
          |___/
```

Goal: a high performance pbrt-style renderer. 

## Features (will have after rewriting)
### Ray tracing kernels:
 Embree and custom SAH BVH.
 Using embree delivers great performance but little achivement, so I' ll write a custom BVH too.

### Light transport algorithm:
Volumetric path tracer </br>
VCM (Unified path sampling)</br>
Multiplexed MLT </br>
Veach style MLT (and its variants)

### Miscellaneous 
We'll have possibly a denoiser and unlikely a blender plugin.

## How to build

Dependencies: Embree 3.0, boost filesystem(not required when on non-Windows).

CMake everything.

## Gallery (Random test images)
![](gallery/living_room.png)

![](gallery/breakfast_room.png)

![](gallery/veach.png)

![](gallery/sibenik.png)

![](gallery/vokselia_spawn_pr0.1.png)

![](gallery/ring.png)

![](gallery/sportsCar.png)

![](gallery/conference.png)

![](gallery/vokselia_spawn.png)

![](gallery/tough_box.png)


