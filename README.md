

# Miyuki Renderer (Refactoring)

```
        _             _    _     __                _
  /\/\ (_)_   _ _   _| | _(_)   /__\ ___ _ __   __| | ___ _ __ ___ _ __
 /    \| | | | | | | | |/ / |  / \/// _ \ '_ \ / _` |/ _ \ '__/ _ \ '__|
/ /\/\ \ | |_| | |_| |   <| | / _  \  __/ | | | (_| |  __/ | |  __/ |
\/    \/_|\__, |\__,_|_|\_\_| \/ \_/\___|_| |_|\__,_|\___|_|  \___|_|
          |___/
```

Goal: a high performance pbrt-style renderer with low hardware requirements but renders great images quickly (sounds like the ultimate goal for computer graphics though :D)



The project is using a design pattern called MVVM. This enables us to decouple core renderer modules with GUI code so that the latter can fully exploit the power of reflection while keeping the former as simple as possible for performance.

## File Structure:

`miyuki/reflection`: A custom RTTI/Reflection/GC/Serialization library. 

`miyuki/graph`: Scene graph. All classes in this folder relies on reflection to provide information to the editor. Upon rendering, the graph is compiled to build the data structure used for rendering.

`miyuki/materials`: All materials.

`miyuki/intergrators`: All integrators.

`miyuki/hw`: Hardware acceleration. OpenGL for now.

`miyuki/core`: Core infrastructure.

`miyuki/utils`,`miyuki/math`: Utility libraries.

`miyuki/ui`: GUI support.

## Features :
- [ ] Custom RTTI/Reflection/GC/Serialization library
  - [x] Intrusive reflection
  - [x] RTTI
  - [x] Basic GC. 
  - [x] Automatic serialization
  - [ ] Cyclic references
- [ ] Editor Support
  - [ ] Cross platform support. Some GUI code calls native Windows API for now.
- [ ] Custom BVH RT kernel
- [ ] Disney BSDF
- [ ] Path tracing & denoiser
- [ ] Hybrid rendering
- [ ] Network rendering 

## How to build

Dependencies: Embree 3.0, boost, OpenEXR.

CMake everything. 

## Gallery 
### Random test images
![](gallery/mc.png)

![](gallery/fireplace_room_8k.png)

![](gallery/fireplace_room.png)

![](gallery/dof.png)

![](gallery/cornell_box.png)

### From previous version

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


