# Miyuki Renderer

This is a hobby project as I learn pbrt. (see http://pbrt.org)

Inspired by https://github.com/JamesGriffin/Path-Tracer, 	https://github.com/fogleman/pt

##  Features:

It is currently using path tracing, bidirectional path tracing, photon mapping (https://graphics.stanford.edu/courses/cs348b-00/course8.pdf)., and sppm. I chose BVH as acceleration structure, with naive split strategy.

It loads .obj files using tiny_obj_loader (the same as my previous projects).

It uses SIMD instructions to intersect multiple triangles at once, resulting in roughly 30%-50% speed boost.

A non-local means filter is used as denoiser (using opencv)

## TODO List:

1. ~~Path tracing with next event estimation~~
2. ~~Photon mapping~~
3. ~~sppm  (Stochastic Progressive Photon Mapping)~~ (see https://www.ci.i.u-tokyo.ac.jp/~hachisuka/ppm.pdf and https://www.ci.i.u-tokyo.ac.jp/~hachisuka/sppm.pdf)
4. ~~Bidirectional path tracing~~
5.  Multiple Importance Sampling (MIS)
6. Metropolis Light Transport
7. Loading model files using GUI
8. Physically based reflection models
9. Better sampler
10. Texture
11. Volumetric
12. Participating media
13. Spectrum
14. Depth of field
15. Refactoring (the GUI module is rubbish)
16. Non-local means filter (I wrote one but it doesn't work)

## Gallery

![](gallery/snapshots-Sun-Dec-30-15-01-56-2018-.png)

![](gallery/snapshots-Sun-Dec-30-15-35-17-2018-.png)
