#include <iostream>
#include <random>
#include <atomic>

#include <pthread.h>
#include <OpenImageIO/imageio.h>

#include <FlatAlg.hpp>

#include "ray.hpp"
#include "hitable.hpp"
#include "sphere.hpp"
#include "rect.hpp"
#include "hitablelist.hpp"
#include "camera.hpp"
#include "material.hpp"
#include "perlin.hpp"
#include "transforms.hpp"
#include "volume.hpp"
#include "triangles.hpp"
#include "utils.hpp"

// Simple experiment with WIDTH = 400, HEIGHT = 225, NUM_SAMPLES = 100 and DEPTH_LIM = 50 showed
// 1 thread -> 56.882 s
// 8 threads -> 16.157 s (speedup of 3.5)

// With dynamic scheduling (threads take new rows until all rows are finished computing)
// 8 threads -> 13.763 s (speedup of 4.13 from original)

// Same as above, again, but using Bounding Volume Hierarchy (BVH): 2.601 s (33.128 s without, maybe due to motion blur)


const int NUM_THREADS = 7;
const int WIDTH = 480, HEIGHT = 360, NUM_SAMPLES = 1, DEPTH_LIM = 5;
// const int WIDTH = 1920, HEIGHT = 1080, NUM_SAMPLES = 256, DEPTH_LIM = 10;

const int MAX_CACHELINE_SIZE = 256;
const int NUM_ELEMENTS_IN_PADDED_ROW = ((WIDTH * sizeof(int) * 3 + MAX_CACHELINE_SIZE - 1) / MAX_CACHELINE_SIZE) * MAX_CACHELINE_SIZE / sizeof(int);

const char* IMAGE_NAME = "perlin.png";

vec3 elementwise_mult(const vec3& v1, const vec3& v2) {
  return vec3(v1[0] * v2[0], v1[1] * v2[1], v1[2] * v2[2]);
}

falg::Vec3 color(const Ray& r, Hitable *world, int depth, unidist& dist) {
  hit_record rec;
  if (world->hit(r, 0.001, MAXFLOAT, rec)) {
    Ray scattered;
    vec3 attenuation;
    vec3 emitted = rec.mat_ptr->emitted(rec.u, rec.v, rec.p);
    if(depth < DEPTH_LIM && rec.mat_ptr->scatter(r, rec, attenuation, scattered, dist)) {
      return emitted + elementwise_mult(attenuation, color(scattered, world, depth + 1, dist));
    } else {
      return emitted;
    }
  }
  else {
    float t = 0.5f * (r.direction().y() + 1.0f);
    // return (1.0 - t) * vec3(1.0f, 1.0f, 1.0f) + t * vec3(0.5, 0.7, 1.0);
    return vec3(0.0f, 0.0f, 0.0f);
  }
}

Hitable* teapot_scene(unidist& dist) {
  int i = 0;
  Hitable** list = new Hitable*[200];

  const int numQuads = 20;


  list[i++] = new TriangleHitable("teapot.obj",
				  // new Lambertian(new ConstantTexture(vec3(0.48, 0.83, 0.53)))
				  new Dielectric(1.5f));

  list[i++] = new Box(vec3(-500.f, -500.0f, -500.f),
                      vec3(500.f, -40.0f, 500.f),
                      new Metal(0.5f * vec3(1.0f, 1.0f, 1.0f), 0.02f));

  for (int j = 0; j < numQuads; j++) {
      float rads = 2 * M_PI * j / (float) numQuads;
      float ang = rads * 180 / M_PI;


      float c0 = (1.0f + cos(rads * 5.0330f + 20)) / 2;
      float c1 = (1.0f + cos(rads * 8.7223f + 10)) / 2;
      float c2 = (1.0f + cos(rads * 10.0239f + 30)) / 2;

      // vec3 color(0.0f, 0.8f, 0.0f);
      vec3 color(c0, c1, c2);

      Hitable *rect = new Rotate(new XYRect(-10.0f, 10.0f, -50.0f, 400.f, -400.0f,
                                               new DiffuseLight(new ConstantTexture(color))),
                                    vec3(0.0f, ang, 0.0f));

      list[i++] = rect;
  }


  // list[i++] = new Sphere(vec3(0.0, 0.0, 0.0), 50.0, new Lambertian(new ConstantTexture(vec3(0.48, 0.83, 0.53))));
  return new BVHNode(list, i, 0, 1, dist);
}

Hitable* finale(unidist& dist) {
  int nb = 20;
  Hitable **list = new Hitable*[30];
  Hitable **boxlist = new Hitable*[10000];
  Hitable **boxlist2 = new Hitable*[10000];
  Material *white = new Lambertian(new ConstantTexture(vec3(0.73, 0.73, 0.73)));
  Material *ground = new Lambertian(new ConstantTexture(vec3(0.48, 0.83, 0.53)));
  int b = 0;
  for(int i = 0; i < nb; i++) {
    for(int j = 0; j < nb; j++) {
      float w = 100;
      float x0 = -1000 + i * w;
      float z0 = -1000 + j * w;
      float y0 = 0;
      float x1 = x0 + w;
      float y1 = 100 * (dist.get() + 0.01);
      float z1 = z0 + w;
      boxlist[b++] = new Box(vec3(x0, y0, z0), vec3(x1, y1, z1), ground);
    }
  }

  int l = 0;
  list[l++] = new BVHNode(boxlist, b, 0, 1, dist);
  Material *light = new DiffuseLight(new ConstantTexture(vec3(7, 7, 7)));
  list[l++] = new XZRect(123, 423, 147, 412, 554, light);
  vec3 center(400, 400, 200);
  list[l++] = new MovingSphere(center, center + vec3(30, 0, 0), 0, 1, 50, new Lambertian(new ConstantTexture(vec3(0.7, 0.3, 0.1))));
  list[l++] = new Sphere(vec3(260, 150, 45), 50, new Dielectric(1.5));
  list[l++] = new Sphere(vec3(0, 150, 145), 50, new Metal(vec3(0.8, 0.8, 0.9), 10.0));

  Hitable *boundary = new Sphere(vec3(360, 150, 145), 70, new Dielectric(1.5));
  list[l++] = boundary;
  list[l++] = new ConstantMedium(boundary, 0.2, new ConstantTexture(vec3(1.0, 1.0, 1.0)), dist);

  boundary = new Sphere(vec3(0, 0, 0), 5000, new Dielectric(1.5));
  list[l++] = new ConstantMedium(boundary, 0.0001, new ConstantTexture(vec3(1.0, 1.0, 1.0)), dist);

  Material *emat = new Lambertian(new ImageTexture("earth.jpeg"));
  list[l++] = new Sphere(vec3(400, 200, 400), 100, emat);
  Texture *pertext = new NoiseTexture(dist, 0.1);
  list[l++] = new Sphere(vec3(220, 280, 300), 80, new Lambertian(pertext));
  int ns = 1000;
  for (int j = 0; j < ns; j++) {
    boxlist2[j] = new Sphere(vec3(165 * dist.get(), 165 * dist.get(), 165 * dist.get()), 10, white);
  }
  list[l++] = new Translate(new Rotate(new BVHNode(boxlist2, ns, 0.0, 1.0, dist), vec3(0, 15, 0)), vec3(-100, 270, 395));
  // return new HitableList(list, l);
  return new BVHNode(list, l, 0, 1, dist);
}

Hitable* cornell_box(unidist& dist) {
  Hitable **list = new Hitable*[9];
  int i = 0;
  Material *red = new Lambertian(new ConstantTexture(vec3(0.65, 0.05, 0.05)));
  Material *white = new Lambertian(new ConstantTexture(vec3(0.73, 0.73, 0.73)));
  Material *green = new Lambertian(new ConstantTexture(vec3(0.12, 0.45, 0.15)));
  Material *light = new DiffuseLight(new ConstantTexture(vec3(15, 15, 15)));

  list[i++] = new FlipNormals(new YZRect(0, 555, 0, 555, 555, green));
  list[i++] = new YZRect(0, 555, 0, 555, 0, red);
  list[i++] = new XZRect(213, 343, 227, 332, 554, light);
  list[i++] = new FlipNormals(new XZRect(0, 555, 0, 555, 555, white));
  list[i++] = new XZRect(0, 555, 0, 555, 0, white);
  list[i++] = new FlipNormals(new XYRect(0, 555, 0, 555, 555, white));

  Hitable *b1 = new Translate(new Rotate(new Box(vec3(0, 0, 0),
						 vec3(165, 165, 165), white),
					 vec3(0, -18, 0)),
			      vec3(130, 0, 65));
  Hitable *b2 = new Translate(new Rotate(new Box(vec3(0, 0, 0),
						 vec3(165, 330, 165), white),
					 vec3(0, 15, 0)),
			      vec3(265, 0, 295));

  list[i++] = new ConstantMedium(b1, 0.01, new ConstantTexture(vec3(1.0, 1.0, 1.0)), dist);
  list[i++] = new ConstantMedium(b2, 0.01, new ConstantTexture(vec3(0.0, 0.0, 0.0)), dist);

  return new HitableList(list, i);
}

Hitable* perlin_spheres(unidist& dist) {
  Texture *pertext = new NoiseTexture(dist, 5.0f);
  Texture *earthtext = new ImageTexture("earth.jpeg");
  Hitable **list = new Hitable*[4];
  list[0] = new Sphere(vec3(0, -1000, 0), 1000, new Lambertian(pertext));
  // list[1] = new Sphere(vec3(0, 2, 0), 2, new Lambertian(pertext));
  list[1] = new Sphere(vec3(0, 2, 0), 2, new Lambertian(earthtext));
  list[2] = new XYRect(3, 5, 1, 3, -2, new DiffuseLight(new ConstantTexture(vec3(4, 4, 4))));
  list[3] = new Sphere(vec3(-8, 8, 8), 3, new DiffuseLight(new ConstantTexture(vec3(4, 4, 4))));
  return new HitableList(list, 4);
}

Hitable* two_spheres(unidist& dist) {
  Texture *checker = new CheckerTexture(new ConstantTexture(vec3(0.2f, 0.3f, 0.1f)),
					new ConstantTexture(vec3(0.9f, 0.9f, 0.9f)));
  int n = 2;
  Hitable **list = new Hitable*[n + 1];
  list[0] = new Sphere(vec3(0, -10, 0), 10, new Lambertian(checker));
  list[1] = new Sphere(vec3(0, 10, 0), 10, new Lambertian(checker));

  return new HitableList(list, 2);

}

Hitable* some_scene(unidist& dist) {
  int n = 500;
  Hitable **list = new Hitable*[n + 1];

  Texture* checks = new CheckerTexture(new ConstantTexture(vec3(0.2f, 0.3f, 0.1f)),
				       new ConstantTexture(vec3(0.9f, 0.9f, 0.9f)));

  list[0] = new Sphere(vec3(0, -1000, 0), 1000, new Lambertian(checks));
  int i = 1;
  for (int a = -11; a < 11; a++) {
    for (int b = -11; b < 11; b++) {
      float choose_mat = dist.get();
      vec3 center(a + 0.7 * dist.get(), 0.2, b + 0.7 * dist.get());
      if ((center - vec3(4, 0.2, 0)).norm() > 0.9) {
	if (choose_mat < 0.8) { // diffuse
	  list[i++] = new MovingSphere(center, center + vec3(0.0f, 0.2 * dist.get(), 0), 0.0, 1.0,
				       0.2, new Lambertian(new ConstantTexture(vec3(dist.get() * dist.get(),
										    dist.get() * dist.get(),
										    dist.get() * dist.get()))));
	} else if (choose_mat < 0.95) { // metal
	  list[i++] = new Sphere(center, 0.2,
				 new Metal(vec3(0.5 * (1 + dist.get()),
						0.5 * (1 + dist.get()),
						0.5 * (1 + dist.get())),
					   0.5 * dist.get()));
	} else {
	  list[i++] = new Sphere(center, 0.2, new Dielectric(1.5));
	}
      }
    }
  }

  list[i++] = new Sphere(vec3(0, 1, 0), 1.0, new Dielectric(1.5));
  list[i++] = new Sphere(vec3(-4, 1, 0), 1.0, new Lambertian(new ConstantTexture(vec3(0.4, 0.2, 0.1))));
  list[i++] = new Sphere(vec3(4, 1, 0), 1.0, new Metal(vec3(0.7, 0.6, 0.5), 0.0));


  return new BVHNode(list, i, 0.0, 1.0, dist);
  // return new HitableList(list, i);
}

struct thread_info {
  std::atomic_int* g_rowcount;
  Hitable *world;
  float *out_array;
  Camera *cam;
};

void* draw_stuff(void* data) {

  unidist dist;

  thread_info info = *(thread_info*)data;

  int curr = info.g_rowcount->fetch_add(1);

  while(curr < HEIGHT) {
    float *out_array = info.out_array + (NUM_ELEMENTS_IN_PADDED_ROW * curr);

    for(int j = 0; j < WIDTH; j++) {
      vec3 col(0.0, 0.0, 0.0);
      for(int s = 0; s < NUM_SAMPLES; s++) {
	float u = (float(j) + dist.get()) / float(WIDTH);
	float v = (float(curr) + dist.get()) / float(HEIGHT);
	Ray r = info.cam->getRay(u, v, dist);

	col += color(r, info.world, 0, dist);
      }

      col /= NUM_SAMPLES;
      col = vec3(sqrt(col[0]), sqrt(col[1]), sqrt(col[2]));

      *(out_array++) = col[0]; // std::max(0, std::min(255, int(255.99 * col[0])));
      *(out_array++) = col[1]; // std::max(0, std::min(255, int(255.99 * col[1])));
      *(out_array++) = col[2]; // std::max(0, std::min(255, int(255.99 * col[2])));
    }

    std::cerr << "Processed row " << curr << " out of " << HEIGHT << std::endl;

    curr = info.g_rowcount->fetch_add(1);
  }


  return NULL;
}


int main() {
  unidist dist;

  // Hitable *world = some_scene(dist);
  // Hitable *world = two_spheres(dist);
  // Hitable *world = perlin_spheres(dist);
  // Hitable *world = cornell_box(dist);
  // Hitable *world = finale(dist);
  Hitable *world = teapot_scene(dist);

  vec3 lookfrom(10, 3, 10);
  vec3 lookat(0, 1, 0);
  float dist_to_focus = (lookfrom-lookat).norm();
  float aperture = 0.05;

  Camera cam(lookfrom * 20.0, lookat, vec3(0, 1, 0), 35, float(WIDTH) / float(HEIGHT),
	     aperture, dist_to_focus,
	     0.0f, 1.0f);
  /* Camera cam(vec3(13, 2, 3),
	     vec3(0, 2, 0),
	     vec3(0, 1, 0),
	     40,
	     float(WIDTH) / float(HEIGHT),
	     0.0,
	     10.0, 0.0f, 1.0f); */
  /* Camera cam(vec3(500, 278, -800), vec3(278, 278, 0),
	     vec3(0, 1, 0), 30.0, float(WIDTH) / float(HEIGHT),
	     0.0, 10.0, 0.0, 1.0); */


  pthread_t threads[NUM_THREADS]; // First never initialized
  thread_info *infos[NUM_THREADS];
  float *result_rows = new float[HEIGHT * NUM_ELEMENTS_IN_PADDED_ROW];
  std::atomic_int global_counter(0);

  for(int i = 0; i < NUM_THREADS; i++) {
    infos[i] = new thread_info;
    infos[i]->world = world;
    infos[i]->cam = &cam;

    infos[i]->out_array = result_rows;
    infos[i]->g_rowcount = &global_counter;
  }

  for(int i = 1; i < NUM_THREADS; i++) {
    if(pthread_create(threads + i, NULL, draw_stuff, infos[i])) {
      std::cerr << "Could not create thread for some reason\n" << std::endl;
      return 0;
    }
  }

  draw_stuff(infos[0]);

  for(int i = 1; i < NUM_THREADS; i++) {
    pthread_join(threads[i], NULL);
  }

  std::unique_ptr<OpenImageIO::ImageOutput> outfile = OpenImageIO::ImageOutput::create(IMAGE_NAME);
  if(!outfile) {
    std::cerr << "Cannot open output file, exiting" << std::endl;
    exit(-1);
  }

  OpenImageIO::ImageSpec spec(WIDTH, HEIGHT, 3, OpenImageIO::TypeDesc::FLOAT);
  outfile->open(IMAGE_NAME, spec);

  // We want to turn image upside down:
  outfile->write_image(OpenImageIO::TypeDesc::FLOAT, result_rows + NUM_ELEMENTS_IN_PADDED_ROW * (HEIGHT - 1),
		       OpenImageIO::AutoStride,
		       - NUM_ELEMENTS_IN_PADDED_ROW * sizeof(float));
  outfile->close();

  for(int i = 0; i < NUM_THREADS; i++) {
    delete infos[i];
  }


  std::cout << "Wrote image to " << IMAGE_NAME << std::endl;

  delete[] result_rows;

  return 0;
}
