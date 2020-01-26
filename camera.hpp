#ifndef INCLUDE_CAMERA_HPP
#define INCLUDE_CAMERA_HPP

#include "utils.hpp"

vec3 random_in_unit_disk(unidist& dist) {
  vec3 p;
  do {
    float f1 = dist.get();
    float f2 = dist.get();
    p = 2.0 * vec3(f1, f2, 0) - vec3(1, 1, 0);
  } while (p * p >= 1.0);
  return p;
}

class Camera {
public:
  Camera(vec3 lookfrom, vec3 lookat, vec3 vup, float vfov, float aspect, float aperture, float focus_dist,
	 float t0, float t1) : time0(t0), time1(t1) {
    lens_radius = aperture / 2;
    
    float theta = vfov * M_PI/180;
    float half_height = tan(theta / 2);
    float half_width = aspect * half_height;

    origin = lookfrom;
    w = (lookfrom - lookat).normalized();
    u = cross(vup, w).normalized();
    v = cross(w, u);
    
    lower_left_corner = origin - half_width * focus_dist * u - half_height * focus_dist * v - focus_dist * w;
    horizontal = 2 * half_width * focus_dist * u;
    vertical = 2 * half_height * focus_dist * v;
  }

  Ray getRay(float s, float t, unidist& dist) {
    vec3 rd = lens_radius * random_in_unit_disk(dist);
    vec3 offset = u * rd.x() + v * rd.y();
    float time = time0 + (time1 - time0) * dist.get();
    return Ray(origin + offset,
	       lower_left_corner + s * horizontal + t * vertical - origin - offset,
	       time);
  }

  vec3 origin;
  vec3 lower_left_corner;
  vec3 horizontal;
  vec3 vertical;
  float lens_radius;

  float time0, time1;
  vec3 u, v, w;
};

#endif // INCLUDE_CAMERA_HPP
