#ifndef INCLUDE_HITABLE_HPP
#define INCLUDE_HITABLE_HPP

#include "ray.hpp"

class Aabb;
// #include "aabb.hpp"

class Material;

struct hit_record {
  float t;
  float u, v;
  vec3 p;
  vec3 normal;
  Material *mat_ptr;
};


class Hitable {
public:
  virtual bool hit(const Ray& r, float t_min, float t_max, hit_record& rec) const = 0;
  virtual bool bounding_box(float t0, float t1, Aabb& box) const = 0;
};

#endif // ndef INCLUDE_HITABLE_HPP
