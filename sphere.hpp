#ifndef INCLUDE_SPHERE_HPP
#define INCLUDE_SPHERE_HPP

#include "hitable.hpp"
#include "material.hpp"
#include "aabb.hpp"

class Sphere : public Hitable {
public:
  Sphere() {}
  Sphere(const vec3& cen, float r, Material* mat) : center(cen), radius(r), mat_ptr(mat) {}
  virtual bool hit(const Ray& r, float tmin, float tmax, hit_record& rec) const;
  virtual bool bounding_box(float t0, float t1, Aabb& box) const;
  vec3 center;
  float radius;
  Material *mat_ptr;
};

bool Sphere::bounding_box(float t0, float t1, Aabb& box) const {
  box = Aabb(center - vec3(radius, radius, radius), center + vec3(radius, radius, radius));
  return true;
}

bool Sphere::hit(const Ray& r, float tmin, float tmax, hit_record& rec) const {
  vec3 oc = r.origin() - center;
  float a = falg::dot(r.direction(), r.direction());
  float b = falg::dot(oc, r.direction());
  float c = falg::dot(oc, oc) - radius * radius;
  float discriminant = b * b - a * c;
  if (discriminant > 0) {
    float temp = (-b -sqrt(b * b - a * c)) / a;
    if (temp < tmax && temp > tmin) {
      rec.t = temp;
      rec.p = r.point_at_parameter(rec.t);
      rec.normal = (rec.p - center) / radius;
      rec.mat_ptr = this->mat_ptr;
      get_sphere_uv((rec.p - center)/radius, &rec.u, &rec.v);
      return true;
    }
    temp = (-b + sqrt(b * b - a * c)) / a;
    if (temp < tmax && temp > tmin) {
      rec.t = temp;
      rec.p = r.point_at_parameter(rec.t);
      rec.normal = (rec.p - center) / radius;
      rec.mat_ptr = this->mat_ptr;
      get_sphere_uv((rec.p - center)/radius, &rec.u, &rec.v);
      return true;
    }
  }
  return false;
}


class MovingSphere: public Hitable {
public:
  MovingSphere();
  MovingSphere(const vec3& cen0, const vec3& cen1, float t0, float t1, float r, Material* mat) :
    center0(cen0), center1(cen1), time0(t0), time1(t1), radius(r), mat_ptr(mat) { }

  virtual bool hit(const Ray& r, float tmin, float tmax, hit_record& rec) const;
  virtual bool bounding_box(float t0, float t1, Aabb& box) const;
  vec3 center(float time) const;
  vec3 center0, center1;
  float time0, time1;
  float radius;
  Material* mat_ptr;

};

vec3 MovingSphere::center(float time) const {
  return this->center0 + (time - time0) / (time1 - time0) * (center1 - center0);
}

bool MovingSphere::hit(const Ray& r, float tmin, float tmax, hit_record& rec) const {
  vec3 curcen = this->center(r.time());
  vec3 oc = r.origin() - curcen;
  float a = falg::dot(r.direction(), r.direction());
  float b = falg::dot(oc, r.direction());
  float c = falg::dot(oc, oc) - radius * radius;
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

bool MovingSphere::bounding_box(float t0, float t1, Aabb& box) const {
  Aabb box0 = Aabb(this->center(t0) - vec3(radius, radius, radius),
		   center(t0) + vec3(radius, radius, radius));
  Aabb box1 = Aabb(this->center(t1) - vec3(radius, radius, radius),
		   center(t1) + vec3(radius, radius, radius));

  box = surrounding_box(box0, box1);
  return true;
}

#endif // ndef INCLUDE_SPHERE_HPP
