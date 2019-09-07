

# MiyukiRenderer

```
        _             _    _     __                _
  /\/\ (_)_   _ _   _| | _(_)   /__\ ___ _ __   __| | ___ _ __ ___ _ __
 /    \| | | | | | | | |/ / |  / \/// _ \ '_ \ / _` |/ _ \ '__/ _ \ '__|
/ /\/\ \ | |_| | |_| |   <| | / _  \  __/ | | | (_| |  __/ | |  __/ |
\/    \/_|\__, |\__,_|_|\_\_| \/ \_/\___|_| |_|\__,_|\___|_|  \___|_|
          |___/
```

Goal: a high performance pbrt-style renderer.


## File Structure:

`miyuki/reflection`: A custom RTTI/Reflection/GC/Serialization library. 

`miyuki/core`: Core renderer code.

`miyuki/core/accelerators`: Acceleration structure interface. 

`miyuki/core/bsdfs`: All bsdfs.

`miyuki/core/materials`: All materials.

`miyuki/core/intergrators`: All integrators.

`miyuki/hw`: Hardware acceleration. OpenGL for now.

`miyuki/utils`,`miyuki/math`, `miyuki/io`: Utility libraries.

`miyuki/ui`: GUI support.

## Features :
- [x] Custom RTTI/Reflection/Serialization library
  - [x] Non-intrusive reflection
  - [x] RTTI
  - [x] Automatic serialization
- [ ] Editor
  - [x] Loading scene files.
  - [x] Importing Wavefront OBJ
  - [ ] Placing mesh files/textures in /mesh, /texture respectively when importing OBJ
  - [ ] Cross platform support. Some GUI code calls native Windows API for now.
  - [ ] Interactive preview
  - [ ] Hardware accelerated preview
- [ ] Custom BVH RT kernel
- [ ] Generic Math Utils
  - [x] Probability Density Function
  - [x] 1D discrete distribution
  - [x] 2D discrete distribution
- [ ] Basic Reflection Model
  - [x] Diffuse (lambertian/Oren-Nayar)
  - [x] Beckmann Microfacet Model
  - [x] GGX Microfacet Model
  - [ ] Specular Transmission/Refelection 
- [ ] Disney BSDF
- [x] Environment Mapping
- [ ] Volume rendering 
- [ ] Path tracing & denoiser
  - [x] Basic Path Tracer with MIS
  - [ ] Generic Path Tracer
  - [ ] Path Guiding
  - [x] Denoiser (BCD)
  - [x] Denoiser (OIDN)
- [ ] NPR
- [ ] Command line rendering
- [ ] Hybrid rendering
- [ ] Network rendering 

## How to build

Dependencies: Embree 3.0, boost, OpenEXR,  GLFW3

CMake everything. 

## Gallery 
## Editor

![](gallery/editor_new.png)

### Random test images

![](gallery/living_room4.png)

![](gallery/future-city.png)

![](gallery/mc_another_view.png)

![](gallery/mc_envmap.png)

![](gallery/volume.png)

![](gallery/envmap.png)

From previous version

![](gallery/mc.png)

![](gallery/fireplace_room_8k.png)

![](gallery/fireplace_room.png)

![](gallery/dof.png)

![](gallery/cornell_box.png)

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


