# raytracing-in-one-weekend
A C++ implementation of Peter Shirley's (et al.) fantastic book series "Ray tracing in one weekend", which can be found [here](https://raytracing.github.io/). Mostly for private/educational use.

## Running the program
Use `./compiled_renderer.o | pnmtopng > output.png` to get a png output, choose the scene by modifying `scene_to_render` in `main.cpp`.

## Example scenes
The final scene of "Raytracing, the next week", rendered with 10k spp and 1920x1920 px:
![A collection of spheres in an isotropic scattering colume, showcasing the featureset of the renderer](./riow_a_week.png)

The final scene of "Raytracing in a weekend", rendered in 1920x1080px, with a HDR skymap:
![A collection of spheres bouncing up and down on a checkered plane](./riow_weekend_hdr.png)

## Features not included in RIOW
* OpenMP is used for parallel rendering, see `Ǹ_THREADS`, make sure to compile with `-fopenmp`.
* Triangles as a primitive, including normal interpolation.
* .obj file import (preliminary .mtl support too)
* HDR environment maps, with importance sampling.

### More features I want to explore
* [CUDA acceleration, with or w/o OptiX](https://developer.nvidia.com/blog/accelerated-ray-tracing-cuda/), very cool.
* Add more hittable primitives: (done!) triangles (and meshes), linear transformations, wireframes (long cylinders?), ray marched SDFs (thus fractals!)
* Loading objects from simple formats like (geometry version done, materials need testing) .obj
* Add BRDF support/more of them. Disney's uber-material BRDF?
* Integrate with my old CUDA-based 2d fluid simulator, ray-march and render!
* Metropolis/path space methods
* Bidirectional path tracing
* HDRi skyboxes (done)

