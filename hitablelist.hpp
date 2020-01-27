#ifndef INCLUDE_HITABLELIST_HPP
#define INCLUDE_HITABLELIST_HPP

#include "hitable.hpp"

class HitableList : public Hitable {
public:
  HitableList() {}
  HitableList(Hitable **l, int n) { list = l; list_size = n; }
  virtual bool hit(const Ray& r, float tmin, float tmax, hit_record& rec) const;
  virtual bool bounding_box(float t0, float t1, Aabb& box) const;
  Hitable **list;
  int list_size;
};

bool HitableList::hit(const Ray& r, float tmin, float tmax, hit_record& rec) const {
  hit_record temp_rec;
  bool hit_anything = false;
  double closest_so_far = tmax;
  for(int i = 0; i < list_size; i++) {
    if(list[i]->hit(r, tmin, closest_so_far, temp_rec)) {
      hit_anything = true;
      closest_so_far = temp_rec.t;
      rec = temp_rec;
    }
  }

  return hit_anything;
}

bool HitableList::bounding_box(float t0, float t1, Aabb& box) const {
  if(this->list_size == 0) {
    return false;
  }

  Aabb bb;
  if(!this->list[0]->bounding_box(t0, t1, bb)) {
    return false;
  }

  for(int i = 1; i < this->list_size; i++) {
    Aabb aa;
    if(!this->list[i]->bounding_box(t0, t1, aa)) {
      return false;
    }

    bb = surrounding_box(aa, bb);
  }

  box = bb;
  return true;
}

#endif // INCLUDE_HITABLELIST_HPP
