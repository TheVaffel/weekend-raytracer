#ifndef __AABB_HPP
#define __AABB_HPP

#include <algorithm> // min, max

#include "ray.hpp"
#include "utils.hpp"

#include "hitable.hpp"

int box_x_compare(const void* a, const void* b) ;
int box_y_compare(const void* a, const void* b) ;
int box_z_compare(const void* a, const void* b) ;

inline float ffmin(float a, float b) { return a < b ? a : b; }
inline float ffmax(float a, float b) { return a > b ? a : b; }

class Aabb {
public:
  Aabb() {}
  Aabb(const vec3& a, const vec3& b) { _min = a; _max = b; }

  vec3 min() const { return _min; }
  vec3 max() const { return _max; }

  /* bool hit(const Ray& r, float tmin, float tmax) const {
    for (int a = 0; a < 3; a++) {
      float t0 = std::min((_min[a] - r.origin()[a]) / r.direction()[a],
			  (_max[a] - r.origin()[a]) / r.direction()[a]);
      float t1 = std::max((_min[a] - r.origin()[a]) / r.direction()[a],
			  (_max[a] - r.origin()[a]) / r.direction()[a]);

      tmin = std::max(t0, tmin);
      tmax = std::min(t1, tmax);

      if (tmax <= tmin) {
	return false;
      }
    }

    return true;
    } */

  inline bool hit(const Ray& r, float tmin, float tmax) const {
    for (int a = 0; a < 3; a++) {
      float invD = 1.0f / r.direction()[a];
      float t0 = (min()[a] - r.origin()[a]) * invD;
      float t1 = (max()[a] - r.origin()[a]) * invD;

      if(invD < 0.0f)
	std::swap(t0, t1);
      tmin = t0 > tmin ? t0 : tmin;
      tmax = t1 < tmax ? t1 : tmax;
      if (tmax <= tmin)
	return false;
    }
    return true;
  }

  vec3 _min;
  vec3 _max;
};

Aabb surrounding_box(Aabb& box0, Aabb& box1) {
  vec3 small(ffmin(box0.min()[0], box1.min()[0]),
	     ffmin(box0.min()[1], box1.min()[1]),
	     ffmin(box0.min()[2], box1.min()[2]));
  vec3 big(ffmax(box0.max()[0], box1.max()[0]),
	   ffmax(box0.max()[1], box1.max()[1]),
	   ffmax(box0.max()[2], box1.max()[2]));
  return Aabb(small, big);
}

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

BVHNode::BVHNode(Hitable **l, int n, float time0, float time1, unidist& dist) {
  int axis = int(3 * dist.get());
  if (axis == 0)
    qsort(l, n, sizeof(Hitable *), box_x_compare);
  else if(axis == 1)
    qsort(l, n, sizeof(Hitable *), box_y_compare);
  else if(axis == 2)
    qsort(l, n, sizeof(Hitable *), box_z_compare);

  if(n == 1) {
    left = right = l[0];
  } else if(n == 2) {
    left = l[0];
    right = l[1];
  } else {
    this->left = new BVHNode(l, n / 2, time0, time1, dist);
    this->right = new BVHNode(l + n / 2, n - n / 2, time0, time1, dist);
  }
  
  Aabb box_left, box_right;
  if(!left->bounding_box(time0, time1, box_left) ||
     !right->bounding_box(time0, time1, box_right)) {
    std::cerr << "No bounding box in BVHNode constructor" << std::endl;
  }
  box = surrounding_box(box_left, box_right);
    
}

int box_x_compare(const void* a, const void* b) {
  Aabb box_left, box_right;
  Hitable* ah = *(Hitable**)a;
  Hitable* bh = *(Hitable**)b;
  if(!ah->bounding_box(0, 0, box_left) || !bh->bounding_box(0, 0, box_right))
    std::cerr << "No bounding box in BVHNode constructor\n";
  if( box_left.min()[0] - box_right.min()[0] < 0.0)
    return -1;
  else
    return 1;
}

int box_y_compare(const void* a, const void* b) {
  Aabb box_left, box_right;
  Hitable* ah = *(Hitable**)a;
  Hitable* bh = *(Hitable**)b;
  if(!ah->bounding_box(0, 0, box_left) || !bh->bounding_box(0, 0, box_right))
    std::cerr << "No bounding box in BVHNode constructor\n";
  if( box_left.min()[1] - box_right.min()[1] < 0.0)
    return -1;
  else
    return 1;
}

int box_z_compare(const void* a, const void* b) {
  Aabb box_left, box_right;
  Hitable* ah = *(Hitable**)a;
  Hitable* bh = *(Hitable**)b;
  if(!ah->bounding_box(0, 0, box_left) || !bh->bounding_box(0, 0, box_right))
    std::cerr << "No bounding box in BVHNode constructor\n";
  if( box_left.min()[2] - box_right.min()[2] < 0.0)
    return -1;
  else
    return 1;
}

bool BVHNode::bounding_box(float t0, float t1, Aabb& b) const {
  b = this->box;
  return true;
}

bool BVHNode::hit(const Ray& r, float tmin, float tmax, hit_record& rec) const {
  if (box.hit(r, tmin, tmax)) {
    hit_record left_rec, right_rec;
    bool hit_left = left->hit(r, tmin, tmax, left_rec);
    bool hit_right = right->hit(r, tmin, tmax, right_rec);

    if (hit_left && hit_right) {
      if(left_rec.t < right_rec.t)
	rec = left_rec;
      else
	rec = right_rec;
      return true;
    } else if (hit_left) {
      rec = left_rec;
      return true;
    } else if (hit_right) {
      rec = right_rec;
      return true;
    }
    else
      return false;
    
  } else return false;
}

#endif // __AABB_HPP
