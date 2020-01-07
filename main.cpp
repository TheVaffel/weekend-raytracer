#include <iostream>
#include <random>
#include <atomic>

#include <pthread.h>
#include <png++/png.hpp>

#include <FlatAlg.hpp>

#include "ray.hpp"
#include "hitable.hpp"
#include "sphere.hpp"
#include "hitablelist.hpp"
#include "camera.hpp"
#include "material.hpp"
#include "utils.hpp"

// Simple experiment with WIDTH = 400, HEIGHT = 225, NUM_SAMPLES = 100 and DEPTH_LIM = 50 showed
// 1 thread -> 56.882 s
// 8 threads -> 16.157 s (speedup of 3.5)

// With dynamic scheduling (threads take new rows until all rows are finished computing)
// 8 threads -> 13.763 s (speedup of 4.13 from original)

const int NUM_THREADS = 8;
const int WIDTH = 400;
const int HEIGHT = 225;
const int NUM_SAMPLES = 100;
const int DEPTH_LIM = 50;

const int MAX_CACHELINE_SIZE = 256;
const int NUM_ELEMENTS_IN_PADDED_ROW = ((WIDTH * sizeof(int) * 3 + MAX_CACHELINE_SIZE - 1) / MAX_CACHELINE_SIZE) * MAX_CACHELINE_SIZE / sizeof(int);

const char* IMAGE_NAME = "out.png";

vec3 elementwise_mult(const vec3& v1, const vec3& v2) {
  return vec3(v1[0] * v2[0], v1[1] * v2[1], v1[2] * v2[2]);
}

Vector3 color(const Ray& r, Hitable *world, int depth, unidist& dist) {
  hit_record rec;
  if (world->hit(r, 0.001, MAXFLOAT, rec)) {
    Ray scattered;
    vec3 attenuation;
    if(depth < DEPTH_LIM && rec.mat_ptr->scatter(r, rec, attenuation, scattered, dist)) {
      return elementwise_mult(attenuation, color(scattered, world, depth + 1, dist));
    } else {
      return vec3(0, 0, 0);
    }
  }
  else {
    vec3 unit_direction = r.direction().normalized();
    float t = 0.5 * (unit_direction[1] + 1.0);
    return (1.0 - t) * vec3(1.0, 1.0, 1.0) + t * vec3(0.5, 0.7, 1.0);
  }
}

Hitable* some_scene(unidist& dist) {
  int n = 500;
  Hitable **list = new Hitable*[n + 1];
  list[0] = new Sphere(vec3(0, -1000, 0), 1000, new Lambertian(vec3(0.5, 0.5, 0.5)));
  int i = 1;
  for (int a = -11; a < 11; a++) {
    for (int b = -11; b < 11; b++) {
      float choose_mat = dist.get();
      vec3 center(a + 0.7 * dist.get(), 0.2, b + 0.7 * dist.get());
      if ((center - vec3(4, 0.2, 0)).norm() > 0.9) {
	if (choose_mat < 0.8) { // diffuse
	  list[i++] = new Sphere(center, 0.2, new Lambertian(vec3(dist.get() * dist.get(),
								  dist.get() * dist.get(),
								  dist.get() * dist.get())));
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
  list[i++] = new Sphere(vec3(-4, 1, 0), 1.0, new Lambertian(vec3(0.4, 0.2, 0.1)));
  list[i++] = new Sphere(vec3(4, 1, 0), 1.0, new Metal(vec3(0.7, 0.6, 0.5), 0.0));
  
  return new HitableList(list, i);
}

struct thread_info {
  std::atomic_int* g_rowcount;
  Hitable *world;
  int *out_array;
  Camera *cam;
};

void* draw_stuff(void* data) {

  unidist dist;
  
  thread_info info = *(thread_info*)data;

  int curr = info.g_rowcount->fetch_add(1);

  while(curr < HEIGHT) {
    int *out_array = info.out_array + (NUM_ELEMENTS_IN_PADDED_ROW * curr);
  
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
      
      *(out_array++) = int(255.99 * col[0]);
      *(out_array++) = int(255.99 * col[1]);
      *(out_array++) = int(255.99 * col[2]);
    }
  
    std::cerr << "Processed row " << curr << " out of " << HEIGHT << std::endl;
    
    curr = info.g_rowcount->fetch_add(1);
  }
  
  
  return NULL;
}


int main() {
  std::cout << "P3\n" << WIDTH << " " << HEIGHT << "\n255\n";

  unidist dist;
  Hitable *world = some_scene(dist);

  vec3 lookfrom(10, 1.5, 5);
  vec3 lookat(0, 1, 0);
  float dist_to_focus = (lookfrom-lookat).norm();
  float aperture = 0.05;
  
  Camera cam(lookfrom, lookat, vec3(0, 1, 0), 35, float(WIDTH) / float(HEIGHT), aperture, dist_to_focus);


  pthread_t threads[NUM_THREADS]; // First never initialized
  thread_info *infos[NUM_THREADS];
  int *result_rows = new int[HEIGHT * NUM_ELEMENTS_IN_PADDED_ROW];
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

  png::image<png::rgb_pixel> out_image(WIDTH, HEIGHT);

  for(int i = 0; i < HEIGHT; i++) {
    for(int j = 0; j < WIDTH; j++) {
      int y = HEIGHT - i - 1;
      out_image[i][j] =
	png::rgb_pixel(result_rows[NUM_ELEMENTS_IN_PADDED_ROW * y + 3 * j + 0],
		       result_rows[NUM_ELEMENTS_IN_PADDED_ROW * y + 3 * j + 1],
		       result_rows[NUM_ELEMENTS_IN_PADDED_ROW * y + 3 * j + 2]);
    }
  }
  
  for(int i = 0; i < NUM_THREADS; i++) {
    delete infos[i];
  }

  out_image.write(IMAGE_NAME);
  
  delete[] result_rows;

  return 0;
}
