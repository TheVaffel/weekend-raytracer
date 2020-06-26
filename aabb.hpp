#ifndef __AABB_HPP
#define __AABB_HPP

#include <algorithm> // min, max
#include <iostream>

#include "ray.hpp"
#include "utils.hpp"

#include "hitable.hpp"

int box_x_compare(const void* a, const void* b) ;
int box_y_compare(const void* a, const void* b) ;
int box_z_compare(const void* a, const void* b) ;

class Aabb {
public:
  Aabb();
  Aabb(const vec3& a, const vec3& b);

  vec3 min() const;
  vec3 max() const;
  bool hit(const Ray& r, float tmin, float tmax) const;

  void add(const vec3& v);

  vec3 _min;
  vec3 _max;
};

Aabb surrounding_box(Aabb& box0, Aabb& box1);

class BVHNode : public Hitable {

public:
  BVHNode() {}
  BVHNode(Hitable **l, int n, float time0, float time1, unidist& dist);

  virtual bool hit(const Ray& r, float tmin, float tmax, hit_record& rec) const;
  virtual bool bounding_box(float t0, float t1, Aabb& box) const;
  Hitable *left;
  Hitable *right;
  Aabb box;
};

#endif // __AABB_HPP
