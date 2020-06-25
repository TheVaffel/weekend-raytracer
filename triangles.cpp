#include "triangles.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tinyobjloader.hpp"

#include "aabb.hpp"

#include <vector>
#include <iostream>

TriangleHitable::TriangleHitable() { }

TriangleHitable::TriangleHitable(const std::string& file_name,
				 Material* mat_ptr) : mat_ptr(mat_ptr) {

  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn;
  std::string err;

  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, file_name.c_str());

  if(!warn.empty()) {
    std::cout << "[tinyobj warning] " << warn << std::endl;
  }

  if(!err.empty()) {
    std::cerr << err << std::endl;
  }

  if (!ret) {
    exit(1);
  }

  for (size_t s = 0; s < shapes.size(); s++) {
    size_t index_offset = 0;
    for(size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
      int fv = shapes[s].mesh.num_face_vertices[f];
      if(fv != 3) {
	std::cerr << "Only triangle faces allowed at this point" << std::endl;
	exit(1);
      }

      
      this->vertices.resize(attrib.vertices.size());
      this->normals.resize(attrib.vertices.size());
      this->uvs.resize(attrib.vertices.size());
	
      for(int i = 0; i < fv; i++) {
	tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + i];

	falg::Vec3 vertex(attrib.vertices[3 * idx.vertex_index + 0],
			  attrib.vertices[3 * idx.vertex_index + 1],
			  attrib.vertices[3 * idx.vertex_index + 2]);
	falg::Vec3 normal(attrib.normals[3 * idx.normal_index + 0],
			  attrib.normals[3 * idx.normal_index + 1],
			  attrib.normals[3 * idx.normal_index + 2]);
	// falg::Vec2 uv(attrib.texcoords[2 * idx.texcoord_index + 0],
	// 	      attrib.texcoords[2 * idx.texcoord_index + 1]);
	
	this->vertices[idx.vertex_index] = vertex;
	this->normals[idx.vertex_index] = normal;
	// this->uvs[idx.vertex_index] = uv;
	
	indices.push_back(idx.vertex_index);
      }

      index_offset += fv;
    }
  }

  this->num_triangles = indices.size() / 3;
}

bool TriangleHitable::hit_triangle(const Ray& r, float tmin, float tmax, hit_record& rec, int ind) const {
  // std::cout << "Calling hit_triangle" << std::endl;
  // The Moller-Trumbore algorithm
  
  using namespace falg;
  Vec3 v0 = this->vertices[this->indices[3 * ind + 0]];
  Vec3 v1 = this->vertices[this->indices[3 * ind + 1]];
  Vec3 v2 = this->vertices[this->indices[3 * ind + 2]];

  Vec3 e1 = v1 - v0;
  Vec3 e2 = v2 - v0;

  Vec3 tvec = r.origin() - v0;
  Vec3 dvec = r.direction();

  Vec3 p = cross(dvec, e2);
  Vec3 q = cross(tvec, e1);

  float det = p * e1;
  if(std::abs(det) <= 1e-7) {
    // Ray and triangle nearly
    // std::cout << "Triangle determinant = " << det << std::endl;
    // std::cout << "p = " << p.str() << ", e1 = " << e1.str() << std::endl;
    // std::cout << "v0 = " << v0.str() << ", v1 = " << v1.str() << ", v2 = " << v2.str() << std::endl;
    return false;
  }
  // std::cout << "Got past determinant check" << std::endl;
  
  Vec3 tuv = (1.0 / det) * Vec3(q * e2, p * tvec, q * dvec);

  float t = tuv[0];
  float u = tuv[1];
  float v = tuv[2];
  
  if(t <= tmin || t >= tmax || (u + v) > 1 || v < 0 || u < 0) {

    /* if(u >= 0 && v >= 0 && u <= 1 && v <= 1) {
      std::cout << "Should be inside triangle, t = " << t << std::endl;
      } */
    // std::cout << "Returning false" << std::endl;
    return false;
  }

  rec.t = t;
  rec.p = r.origin() + dvec * t;
  rec.normal = cross(e2, e1).normalized();
  rec.mat_ptr = this->mat_ptr;
  rec.u = u;
  rec.v = v;
  // std::cout << "Returned true!!" << std::endl;
  return true;
}

bool TriangleHitable::hit(const Ray& r, float tmin, float tmax, hit_record& rec) const {
  // Naive intersection implementation
  bool hit_anything = false;
  for(int i = 0; i < this->num_triangles; i++) {
    if(hit_triangle(r, tmin, tmax, rec, i)) {
      tmax = rec.t;
      hit_anything = true;
    }
  }

  return hit_anything;
}

bool TriangleHitable::bounding_box(float t0, float t1, Aabb& box) const {
  falg::Vec3 minv = vertices[0];
  falg::Vec3 maxv = vertices[1];
  for(size_t i = 1; i < vertices.size(); i++) {
    for(int j = 0; j < 3; j++) {
      minv[j] = std::min(vertices[i][j], minv[j]);
      maxv[j] = std::max(vertices[i][j], maxv[j]);
    }
  }

  box = Aabb(minv, maxv);
  std::cout << "Min vec = " << minv.str() << ", "
	    << "max vec = " << maxv.str() << std::endl;
  return true;
}
