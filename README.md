# Weekend Ray Tracer

Ray Tracer based on the [Ray Tracer in One Weekend booklet](http://www.realtimerendering.com/raytracing/Ray%20Tracing%20in%20a%20Weekend.pdf).

### Differences from original
- Multithreaded with `pthreads`
- Using [HConLib](https://github.com/TheVaffel/HConLib) (`FlatAlg`) for linear algebra
- Outputs in '.png' format using `libpng++`

### Usage

Run the compile script, and if everything is in order (you need `libpng++` and `pthread` development packages), you should be able to run 

### Potential issues / improvements
- All configuration (image width, height and output image etc.) is done inside the `main.cpp` file, which means the program must be recompiled between each change.
- Only spheres can be rendered at this point. I mean, if you look at atoms as tiny spheres of certain colors (which they really aren't) and assume perfect precision and infinite compute power (which I am gonna go ahead and assume that you don't have), this is enough. Otherwise, adding new components would be nice.
- Support triangular formats (with fancy file I/O maybe?)
- The code is inconsistent in which of the terms `vec3` (from the booklet) and `Vector3` (standard in `FlatAlg`) to use.