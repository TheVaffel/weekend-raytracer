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


class MovingSphere: public Hitable {
public:
  MovingSphere();
  MovingSphere(Vector3 cen0, Vector3 cen1, float t0, float t1, float r, Material* mat) :
    center0(cen0), center1(cen1), time0(t0), time1(t1), radius(r), mat_ptr(mat) { }

  virtual bool hit(const Ray& r, float tmin, float tmax, hit_record& rec) const;
  Vector3 center(float time) const;
  Vector3 center0, center1;
  float time0, time1;
  float radius;
  Material* mat_ptr;
  
};

Vector3 MovingSphere::center(float time) const {
  return this->center0 + (time - time0) / (time1 - time0) * (center1 - center0);
}

bool MovingSphere::hit(const Ray& r, float tmin, float tmax, hit_record& rec) const {
  vec3 curcen = this->center(r.time());
  vec3 oc = r.origin() - curcen;
  float a = r.direction() * r.direction();
  float b = oc * r.direction();
  float c = oc * oc - radius * radius;
  float discriminant = b * b - a * c;
  if (discriminant > 0) {
    float temp = (-b -sqrt(b * b - a * c)) / a;
    if (temp < tmax && temp > tmin) {
      rec.t = temp;
      rec.p = r.point_at_parameter(rec.t);
      rec.normal = (rec.p - curcen) / radius;
      rec.mat_ptr = this->mat_ptr;
      return true;
    }
    temp = (-b + sqrt(b * b - a * c)) / a;
    if (temp < tmax && temp > tmin) {
      rec.t = temp;
      rec.p = r.point_at_parameter(rec.t);
      rec.normal = (rec.p - curcen) / radius;
      rec.mat_ptr = this->mat_ptr;
      return true;
    }
  }
  return false;
}

#endif // ndef INCLUDE_SPHERE_HPP
