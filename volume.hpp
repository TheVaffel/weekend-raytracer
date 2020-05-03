#ifndef _VOLUME_HPP
#define _VOLUME_HPP


#include "hitable.hpp"
#include "texture.hpp"

class ConstantMedium : public Hitable {
public:
  ConstantMedium(Hitable *p, float d, Texture* tex, unidist& dist) : boundary(p), density(d), dist(dist) {
    this->phase_function = new Isotropic(tex);
  }
  
  virtual bool hit(const Ray& r, float t_min, float t_max, hit_record& rec) const;
  virtual bool bounding_box(float t0, float t1, Aabb& box) const {
    return boundary->bounding_box(t0, t1, box);
  }

  Hitable *boundary;
  float density;
  Material *phase_function;
  unidist& dist;
};

bool ConstantMedium::hit(const Ray& r, float t_min, float t_max, hit_record& rec) const {
  hit_record rec1, rec2;
  if(boundary->hit(r, -1e10, 1e10, rec1)) {
    if(boundary->hit(r, rec1.t + 1e-4, 1e10, rec2)) {
      if(rec1.t < t_min)
	rec1.t = t_min;
      if(rec2.t > t_max)
	rec2.t = t_max;
      if(rec1.t >= rec2.t)
	return false;
      if(rec1.t < 0)
	rec1.t = 0;
      float distance_inside_boundary = (rec2.t - rec1.t) * r.direction().norm();
      float hit_distance  = -(1.0 / density) * log(dist.get());
      if (hit_distance < distance_inside_boundary) {
	rec.t = rec1.t + hit_distance / r.direction().norm();
	rec.p = r.point_at_parameter(rec.t);
	rec.normal = vec3(1, 0, 0);
	rec.mat_ptr = this->phase_function;
	return true;
      }
    }
  }
  return false;
}

#endif // _VOLUME_HPP
