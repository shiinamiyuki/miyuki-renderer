Miyuki Renderer

This is a hobby project as I learn pbrt. (see http://pbrt.org)

Inspired by https://github.com/JamesGriffin/Path-Tracer, 	https://github.com/fogleman/pt

SPPM and PSSMLT are amazing and we need more of these.

Mark these words: 'Don't ever attempt to write complicated GPU code if you only have limit time' .

我再写一次OpenCL我是狗

##  Features:

It is currently using path tracing, bidirectional path tracing, photon mapping (https://graphics.stanford.edu/courses/cs348b-00/course8.pdf)., and sppm. I chose BVH as acceleration structure, with naive split strategy.

It loads .obj files using tiny_obj_loader (the same as my previous projects).

It uses SIMD instructions to intersect multiple triangles at once, resulting in roughly 30%-50% speed boost.

A non-local means filter is used as denoiser (using opencv)

## TODO List:
0. Rewrite a CPU version using Embree and replace that shit GUI with ImGUI
1. ~~Path tracing with next event estimation~~
2. ~~Photon mapping~~
3. ~~sppm  (Stochastic Progressive Photon Mapping)~~ (see https://www.ci.i.u-tokyo.ac.jp/~hachisuka/ppm.pdf and https://www.ci.i.u-tokyo.ac.jp/~hachisuka/sppm.pdf) (write a GPU version)
4. ~~Bidirectional path tracing~~
5. ~~Multiple Importance Sampling (MIS)~~
6. Faster BVH traversal
7. Metropolis Light Transport (Possibly PSSMLT)
8. Vertex Connection and Merging (VCM)
9. Rasterizer (for fun)
10. Toon shading
11. Instant radiosity
12. Loading model files using GUI
13. Physically based reflection models
14. Grids
15. Surface heuristic
16. Better sampler
17. Texture
18. Volumetric
19. Participating media
20. Spectrum
21. Depth of field
22. Refactoring (the GUI module is rubbish)
23. Non-local means filter (I wrote one but it doesn't work)
24. Programmable shader (possibly a homemade JIT compiler)

## Gallery



![](gallery/snapshots-Sun-Jan--6-20-01-23-2019-.png)

Glossy surface rendered using MIS

![](gallery/snapshots-Sun-Jan--6-21-42-30-2019-.png)

From old CPU version:

![](gallery/snapshots-Thu-Jan--3-05-12-31-2019-.png)

![](gallery/snapshots-Sun-Dec-30-15-01-56-2018-.png)

![](gallery/snapshots-Tue-Jan--1-17-09-22-2019-.png)
