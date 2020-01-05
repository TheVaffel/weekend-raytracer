#ifndef INCLUDE_HITABLE_HPP
#define INCLUDE_HITABLE_HPP

#include "ray.hpp"

class Material;

struct hit_record {
  float t;
  vec3 p;
  vec3 normal;
  Material *mat_ptr;
};


class Hitable {
public:
  virtual bool hit(const Ray& r, float t_min, float t_max, hit_record& rec) const = 0;
};

#endif // ndef INCLUDE_HITABLE_HPP
