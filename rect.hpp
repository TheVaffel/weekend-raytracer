

#ifndef _RECT_HPP
#define _RECT_HPP

#include "hitable.hpp"
#include "hitablelist.hpp"
#include "material.hpp"
#include "transforms.hpp"

class XYRect : public Hitable {
public:
  XYRect() {}
  XYRect(float x0, float x1, float y0, float y1, float k, Material* mat) : x0(x0), x1(x1), y0(y0), y1(y1), k(k), mp(mat) { }
  virtual bool hit(const Ray& r, float t0, float t1, hit_record& rec) const;
  virtual bool bounding_box(float t0, float t1, Aabb& box) const {
    box = Aabb(vec3(x0, y0, k - 0.0001), vec3(x1, y1, k + 0.0001));
    return true;
  }
  
  float x0, x1, y0, y1, k;
  Material* mp;

};

bool XYRect::hit(const Ray& r, float t0, float t1, hit_record& rec) const {
  float t = (k - r.origin().z()) / r.direction().z();
  if (t < t0 || t > t1) {
    return false;
  }
  float x = r.origin().x() + t * r.direction().x();
  float y = r.origin().y() + t * r.direction().y();
  if(x < x0 || x > x1 || y < y0 || y > y1) {
    return false;
  }

  rec.u = (x - x0) / (x1 - x0);
  rec.v = (y - y0) / (y1 - y0);
  rec.t = t;
  rec.mat_ptr = mp;
  rec.p = r.point_at_parameter(t);
  rec.normal = vec3(0, 0, 1);
  return true;
}



class XZRect : public Hitable {
public:
  XZRect() {}
  XZRect(float x0, float x1, float z0, float z1, float k, Material* mat) : x0(x0), x1(x1), z0(z0), z1(z1), k(k), mp(mat) { }
  virtual bool hit(const Ray& r, float t0, float t1, hit_record& rec) const;
  virtual bool bounding_box(float t0, float t1, Aabb& box) const {
    box = Aabb(vec3(x0, k - 0.0001, z0), vec3(x1, k + 0.0001, z1));
    return true;
  }
  
  float x0, x1, z0, z1, k;
  Material* mp;

};

bool XZRect::hit(const Ray& r, float t0, float t1, hit_record& rec) const {
  float t = (k - r.origin().y()) / r.direction().y();
  if (t < t0 || t > t1) {
    return false;
  }
  float x = r.origin().x() + t * r.direction().x();
  float z = r.origin().z() + t * r.direction().z();
  if(x < x0 || x > x1 || z < z0 || z > z1) {
    return false;
  }

  rec.u = (x - x0) / (x1 - x0);
  rec.v = (z - z0) / (z1 - z0);
  rec.t = t;
  rec.mat_ptr = mp;
  rec.p = r.point_at_parameter(t);
  rec.normal = vec3(0, 1, 0);
  return true;
}




class YZRect : public Hitable {
public:
  YZRect() {}
  YZRect(float y0, float y1, float z0, float z1, float k, Material* mat) : y0(y0), y1(y1), z0(z0), z1(z1), k(k), mp(mat) { }
  virtual bool hit(const Ray& r, float t0, float t1, hit_record& rec) const;
  virtual bool bounding_box(float t0, float t1, Aabb& box) const {
    box = Aabb(vec3(k - 0.0001, y0, z0), vec3(k + 0.0001, y1, z1));
    return true;
  }
  
  float y0, y1, z0, z1, k;
  Material* mp;

};

bool YZRect::hit(const Ray& r, float t0, float t1, hit_record& rec) const {
  float t = (k - r.origin().x()) / r.direction().x();
  if (t < t0 || t > t1) {
    return false;
  }
  float y = r.origin().y() + t * r.direction().y();
  float z = r.origin().z() + t * r.direction().z();
  if(y < y0 || y > y1 || z < z0 || z > z1) {
    return false;
  }

  rec.u = (y - y0) / (y1 - y0);
  rec.v = (z - z0) / (z1 - z0);
  rec.t = t;
  rec.mat_ptr = mp;
  rec.p = r.point_at_parameter(t);
  rec.normal = vec3(1, 0, 0);
  return true;
}

class Box : public Hitable {
public:
  Box() {}
  Box(const vec3& p0, const vec3& p1, Material *ptr);
  virtual bool hit(const Ray& r, float t0, float t1, hit_record& rec) const;
  virtual bool bounding_box(float t0, float t1, Aabb& box) const {
    box = Aabb(pmin, pmax);
    return true;
  }
  vec3 pmin, pmax;
  Hitable *list_ptr;
};

Box::Box(const vec3& p0, const vec3& p1, Material *ptr) {
  pmin = p0;
  pmax = p1;

  Hitable **list = new Hitable*[6];
  list[0] = new XYRect(p0.x(), p1.x(), p0.y(), p1.y(), p1.z(), ptr);
  list[1] = new FlipNormals(new XYRect(p0.x(), p1.x(), p0.y(), p1.y(), p0.z(), ptr));
  list[2] = new XZRect(p0.x(), p1.x(), p0.z(), p1.z(), p1.y(), ptr);
  list[3] = new FlipNormals(new XZRect(p0.x(), p1.x(), p0.z(), p1.z(), p0.y(), ptr));
  list[4] = new YZRect(p0.y(), p1.y(), p0.z(), p1.z(), p1.x(), ptr);
  list[5] = new FlipNormals(new YZRect(p0.y(), p1.y(), p0.z(), p1.z(), p0.x(), ptr));
  list_ptr = new HitableList(list, 6);
}

bool Box::hit(const Ray& r, float t0, float t1, hit_record& rec) const {
  return list_ptr->hit(r, t0, t1, rec);
}

#endif // _RECT_HPP
