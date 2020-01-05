#ifndef INCLUDE_SPHERE_HPP
#define INCLUDE_SPHERE_HPP

#include "hitable.hpp"
#include "material.hpp"

class Sphere : public Hitable {
public:
  Sphere() {}
  Sphere(const vec3& cen, float r, Material* mat) : center(cen), radius(r), mat_ptr(mat) {}
  virtual bool hit(const Ray& r, float tmin, float tmax, hit_record& rec) const;
  vec3 center;
  float radius;
  Material *mat_ptr;
};

bool Sphere::hit(const Ray& r, float tmin, float tmax, hit_record& rec) const {
  vec3 oc = r.origin() - center;
  float a = r.direction() * r.direction();
  float b = oc * r.direction();
  float c = oc * oc - radius * radius;
  float discriminant = b * b - a * c;
  if (discriminant > 0) {
    float temp = (-b -sqrt(b * b - a * c)) / a;
    if (temp < tmax && temp > tmin) {
      rec.t = temp;
      rec.p = r.point_at_parameter(rec.t);
      rec.normal = (rec.p - center) / radius;
      rec.mat_ptr = this->mat_ptr;
      return true;
    }
    temp = (-b + sqrt(b * b - a * c)) / a;
    if (temp < tmax && temp > tmin) {
      rec.t = temp;
      rec.p = r.point_at_parameter(rec.t);
      rec.normal = (rec.p - center) / radius;
      rec.mat_ptr = this->mat_ptr;
      return true;
    }
  }
  return false;
}

#endif // ndef INCLUDE_SPHERE_HPP
