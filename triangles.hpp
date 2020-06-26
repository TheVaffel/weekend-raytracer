#ifndef INCLUDE_TRIANGLES_HPP
#define INCLUDE_TRIANGLES_HPP

#include "ray.hpp"
#include "hitable.hpp"
#include "aabb.hpp"

#include <vector>

class TriangleHitable;

class TriangleBVH {
  TriangleBVH *l = nullptr, *r = nullptr;
  Aabb box;
  
  int num_inds = 0;
  int* inds = nullptr;

  friend class TriangleHitable;
};

class TriangleHitable : public Hitable {
  std::vector<falg::Vec3> vertices;
  std::vector<falg::Vec2> uvs;
  std::vector<falg::Vec3> normals;
  std::vector<int> indices;

  std::vector<float> diffuse_texture;

  int num_triangles;
  
  Material *mat_ptr;
  TriangleBVH * bvh_root;
  
public:
  TriangleHitable();
  TriangleHitable(const std::string& file_name, Material *mat_ptr);

  ~TriangleHitable();

  void print_bvh(TriangleBVH* tri, int depth);
  
  TriangleBVH* construct_bvh_tree(const std::vector<int>& inds);
  static void deconstruct_bvh_tree(TriangleBVH* node);

  void fill_triangle_min_coord(std::vector<std::pair<float, int> >& min_coords,
			       const std::vector<int>& inds,
			       int coord);

  void compute_separating_boxes(const std::vector<std::pair<float, int> >& vals,
				Aabb& b0,
				Aabb& b1);

  Aabb compute_aabb(const std::vector<int>& inds) const;
  float separation_score(const Aabb& b0, const Aabb& b1);
  
  
  
  bool hit_triangle_bvh(TriangleBVH* node, const Ray& r,
			float tmin, float tmax, hit_record& rec, int depth) const;
  bool hit_triangle(const Ray& r, float tmin, float tmax, hit_record& rec, int ind) const;
  virtual bool hit(const Ray& r, float tmin, float tmax, hit_record& rec) const;
  virtual bool bounding_box(float t0, float t1, Aabb& box) const;
};

#endif // INCLUDE_TRIANGLES_HPP
