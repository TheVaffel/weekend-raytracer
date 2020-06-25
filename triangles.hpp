#ifndef INCLUDE_TRIANGLES_HPP
#define INCLUDE_TRIANGLES_HPP

#include "ray.hpp"
#include "hitable.hpp"

#include <vector>

class TriangleHitable : public Hitable {
  std::vector<falg::Vec3> vertices;
  std::vector<falg::Vec2> uvs;
  std::vector<falg::Vec3> normals;
  std::vector<int> indices;

  std::vector<float> diffuse_texture;

  int num_triangles;
  
  Material *mat_ptr;
public:
  TriangleHitable();
  TriangleHitable(const std::string& file_name, Material *mat_ptr);

  bool hit_triangle(const Ray& r, float tmin, float tmax, hit_record& rec, int ind) const;
  virtual bool hit(const Ray& r, float tmin, float tmax, hit_record& rec) const;
  virtual bool bounding_box(float t0, float t1, Aabb& box) const;
};

#endif // INCLUDE_TRIANGLES_HPP
