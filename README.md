# Weekend Ray Tracer

Ray Tracer based on the [Ray Tracer in One Weekend booklet](http://www.realtimerendering.com/raytracing/Ray%20Tracing%20in%20a%20Weekend.pdf).

### Example Result

Yes, it's the same scene as the one from the booklet, recreated with this ray tracer

![alt text](out.png)

### Differences from original
- Multithreaded with `pthreads`
- Using [HConLib](https://github.com/TheVaffel/HConLib) (`FlatAlg`) for linear algebra
- Outputs in `.png` format using `libpng++`

#### Features from [Ray Tracing the Next Week](http://www.realtimerendering.com/raytracing/Ray%20Tracing_%20The%20Next%20Week.pdf)

- Motion blur
- Bounding Volume Hierarchy (BVH)

### Some Optimization Measurements

- Original serial program: 56.882 s
- Parallelized with 8 threads, each thread is pre-allocated a chunk of contiguous rows: 16.157 s (speedup of 3.5)
- Parallelized with 8 threads, each thread continues to fetch new rows to compute until all rows are taken: 13.763 s (speedup of 4.13 from original)
- After motion blur was implemented, I measure 33.128 s on same laptop and same configuration as above. (Might also be because laptop is unplugged and low on power).
- In the exact same setting as the above measurement, the implementation with BVH measures 2.652 s, a speedup of 12.49 (!)

Measurements were done using an i7 with 8 cores (including hyperthreading) on a five year-old laptop on battery.
It produced an image similar to the example above with dimensions 400 x 225, 100 samples per pixel and a depth limit of 50.
As can be seen in the picture, some rows (like the one in the middle) have way more complex light interactions than others (like the one on top of the image). This is probably the largest reason that dynamically fetching new rows balances the work better than just pre-assigning a chunk to each thread, since some chunks are bound to be more expensive than others.

### Usage

Run the compile script, and if everything is in order (you need `libpng++` and `pthread` development packages), you should be able to run 

### Potential issues / improvements

- All configuration (image width, height and output image etc.) is done inside the `main.cpp` file, which means the program must be recompiled between each change.
- Only spheres can be rendered at this point. I mean, if you look at atoms as tiny spheres of certain colors (which they really aren't) and assume perfect precision and infinite compute power (which I am gonna go ahead and assume that you don't have), this is enough. Otherwise, adding new components would be nice.
- Support triangular formats (with fancy file I/O maybe?)
- The code is inconsistent in which of the terms `vec3` (from the booklet) and `Vector3` (standard in `FlatAlg`) to use.
