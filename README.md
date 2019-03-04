

# Miyuki Renderer(WIP)

```
        _             _    _     __                _
  /\/\ (_)_   _ _   _| | _(_)   /__\ ___ _ __   __| | ___ _ __ ___ _ __
 /    \| | | | | | | | |/ / |  / \/// _ \ '_ \ / _` |/ _ \ '__/ _ \ '__|
/ /\/\ \ | |_| | |_| |   <| | / _  \  __/ | | | (_| |  __/ | |  __/ |
\/    \/_|\__, |\__,_|_|\_\_| \/ \_/\___|_| |_|\__,_|\___|_|  \___|_|
          |___/
```
<b>抄书到此为止，接下来项目将按自己理解重写</b>
<b>Currently under rewriting</b>

Goal: a high performance pbrt-style renderer. 

## Features (will have after rewriting)
### Ray tracing kernels:
 Embree and custom SAH BVH.
 Using embree delievers great performance but little achivement, so I' ll write a custom BVH too.

### Light transport algorithm:
Volumetric path tracer (Unless volpath is completely implemented, I won't move on to others)</br>
VCM (Unified path sampling)</br>
Multiplexed MLT (It's just 100 lines of code, why not?)</br>
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


