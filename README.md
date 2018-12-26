# Miyuki Renderer

This is a hobby project as I learn pbrt. (see http://pbrt.org)

Inspired by https://github.com/JamesGriffin/Path-Tracer, 	https://github.com/fogleman/pt

图形学上不会再开新坑了，这是最后一个项目（只是可能会有点大）。如果我够无聊会把我的脚本引擎给整合进来来处理场景文件(现在是用的十分钟撸的XML parser)

##  Features:

It is currently using path tracing as well as photon mapping. I chose BVH as acceleration structure, with naive split strategy.

It loads .obj files using tiny_obj_loader (the same as my previous projects).

It uses SIMD instructions to intersect multiple triangles at once, resulting in roughly 30%-50% speed boost.

A non-local means filter is used as denoiser (using opencv)

## TODO List:

1. ~~Path tracing with next event estimation~~
2. ~~Photon mapping~~
3. sppm  (Stochastic Progressive Photon Mapping)
4. Loading model files using GUI
5. Physically based reflection models
6. Better sampler
7. Texture
8. Volumetric
9. Participating media
10. Spectrum
11. Depth of field
12.  Refactoring (the GUI module is rubbish)
13. Non-local means filter (I wrote one but it doesn't work)

## Gallery

![](gallery/xmas.png)

![](gallery/gopher.png)

![](gallery/cornell.png)





