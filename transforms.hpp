#ifndef _TRANSFORMS_HPP
#define _TRANSFORMS_HPP

#include "hitable.hpp"
#include "aabb.hpp"

class FlipNormals : public Hitable {
public:
  FlipNormals(Hitable *p) : ptr(p) {}
  virtual bool hit(const Ray& r, float t_min, float t_max, hit_record& rec) const {
    if(ptr->hit(r, t_min, t_max, rec)) {
      rec.normal = -rec.normal;
      return true;
    }
    else
      return false;
  }

  virtual bool bounding_box(float t0, float t1, Aabb& box) const {
    return ptr->bounding_box(t0, t1, box);
  }

  Hitable *ptr;
};

class Translate : public Hitable {
public:
  Translate(Hitable *p, const vec3& displacement) : ptr(p), offset(displacement) {}
  virtual bool hit(const Ray& r, float t_min, float t_max, hit_record& rec) const;
  virtual bool bounding_box(float t0, float t1, Aabb& box) const;
  Hitable *ptr;
  vec3 offset;
};

bool Translate::hit(const Ray& r, float t_min, float t_max, hit_record& rec) const {
  Ray moved_r(r.origin() - this->offset, r.direction(), r.time());
  if (ptr->hit(moved_r, t_min, t_max, rec)) {
    rec.p += offset;
    return true;
  } else {
    return false;
  }
}

bool Translate::bounding_box(float t0, float t1, Aabb& box) const {
  if (ptr->bounding_box(t0, t1, box)) {
    box = Aabb(box.min() + this->offset, box.max() + offset);
    return true;
  } else {
    return false;
  }
}

class Rotate : public Hitable {
public:
  Rotate(Hitable *p, const vec3& angles);
  virtual bool hit(const Ray& r, float t_min, float t_max, hit_record& rec) const;
  virtual bool bounding_box(float t0, float t1, Aabb& box) const {
    box = bbox;
    return hasbox;
  };

  Hitable *ptr;
  vec3 cos_rotation;
  vec3 sin_rotation;
  bool hasbox;
  Aabb bbox;
};

vec3 rot_ax(const vec3& point, const vec3& cosrot, const vec3& sinrot, int ax) {
  vec3 res;
  if(ax == 0) {
    res = vec3(point[0],
	       cosrot[0] * point[1] - sinrot[0] * point[2],
	       sinrot[0] * point[1] + cosrot[0] * point[2]);
  } else if (ax == 1) {
    res = vec3(cosrot[1] * point[0] + sinrot[1] * point[2],
	       point[1],
	       - sinrot[1] * point[0] + cosrot[1] * point[2]);
  } else if (ax == 2) {
    res = vec3(cosrot[2] * point[0] - sinrot[2] * point[1],
	       sinrot[2] * point[0] + cosrot[2] * point[1],
	       point[2]);
  }

  return res;
}

vec3 rotate_axes(const vec3& point, const vec3& cosrot, const vec3& sinrot) {
  vec3 p = point;
  p = rot_ax(p, cosrot, sinrot, 0);
  p = rot_ax(p, cosrot, sinrot, 1);
  p = rot_ax(p, cosrot, sinrot, 2);

  return p;
}

vec3 rotate_axes_inv(const vec3& point, const vec3& cosrot, const vec3& sinrot) {
  // Inverse rotation means negative sine component and reversed rotation order
  vec3 isin = -sinrot;
  vec3 p = point;
  p = rot_ax(p, cosrot, isin, 2);
  p = rot_ax(p, cosrot, isin, 1);
  p = rot_ax(p, cosrot, isin, 0);

  return p;
}

Rotate::Rotate(Hitable* p, const vec3& angles) : ptr(p) {
  vec3 radians =  (F_PI / 180.0) * angles;
  cos_rotation = vec3(cos(radians[0]), cos(radians[1]), cos(radians[2]));
  sin_rotation = vec3(sin(radians[0]), sin(radians[1]), sin(radians[2]));
  hasbox = ptr->bounding_box(0, 1, bbox);

  vec3 min(1e10, 1e10, 1e10);
  vec3 max(-1e10, -1e10, -1e10);

  for(int i = 0; i < 2; i++) {
    for(int j = 0; j < 2; j++) {
      for(int k = 0; k < 2; k++) {
	float x = i * bbox.max().x() + (1 - i) * bbox.min().x();
	float y = j * bbox.max().y() + (1 - j) * bbox.min().y();
	float z = k * bbox.max().z() + (1 - k) * bbox.min().z();

	vec3 rot = rotate_axes(vec3(x, y, z), cos_rotation, sin_rotation);

	for(int c = 0; c < 3; c++) {
	  if(rot[c] > max[c] ) {
	    max[c] = rot[c];
	  }
	  if(rot[c] < min[c] ) {
	    min[c] = rot[c];
	  }
	}
      }
    }
  }
  bbox = Aabb(min, max);
}

bool Rotate::hit(const Ray& r, float t_min, float t_max, hit_record& rec) const {
  vec3 origin = r.origin();
  vec3 direction = r.direction();

  origin = rotate_axes_inv(origin, cos_rotation, sin_rotation);
  direction = rotate_axes_inv(direction, cos_rotation, sin_rotation);

  Ray rotated(origin, direction, r.time());


  if(ptr->hit(rotated, t_min, t_max, rec)) {
    vec3 p = rec.p;
    vec3 normal = rec.normal;
    p = rotate_axes(p, cos_rotation, sin_rotation);
    normal = rotate_axes(normal, cos_rotation, sin_rotation);

    rec.p = p;
    rec.normal = normal;
    return true;
  } else {
    return false;
  }
}

#endif // _TRANSFORMS_HPP
