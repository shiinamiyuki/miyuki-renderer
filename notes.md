# Miyuki Renderer

```
        _             _    _     __                _
  /\/\ (_)_   _ _   _| | _(_)   /__\ ___ _ __   __| | ___ _ __ ___ _ __
 /    \| | | | | | | | |/ / |  / \/// _ \ '_ \ / _` |/ _ \ '__/ _ \ '__|
/ /\/\ \ | |_| | |_| |   <| | / _  \  __/ | | | (_| |  __/ | |  __/ |
\/    \/_|\__, |\__,_|_|\_\_| \/ \_/\___|_| |_|\__,_|\___|_|  \___|_|
          |___/
```

# Jan, 2019

We have implemented basic path tracer and a pssmlt. The latter has tons of issues including potential memory leak and terrible performance.

As for now, rather than implement other light transport algorithms, I believe it's more important to refine the material, bxdf and light source interfaces.

Next things to do: 

1. Distinguish geometric normal with shading normal. 

2. BRDF for GGX microfacet model. (The current implementation is not energy conserving)

3. Refactor `Transform` class so that it uses transform matrix entirely

4. Other glossy model

5. Fix possible issues with texture mapping

6. Environmental mapping
7. Unify naming. (Like using `normal` instead of `norm` through out the program)

