#include "triangles.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tinyobjloader.hpp"

#include "aabb.hpp"

#include <vector>
#include <iostream>
#include <iomanip>

void TriangleHitable::print_bvh(TriangleBVH* bvh, int depth = 0) {
  if(bvh->num_inds) {
    std::cout << std::setw(depth) << "\tnum_indices = " << bvh->num_inds << ", first = " << bvh->inds[0] << ", last = " << bvh->inds[bvh->num_inds - 1] << std::endl;
  } else {
    std::cout << std::setw(depth) << "\tNon-leaf" << std::endl;
  }

  if(bvh->l) {
    print_bvh(bvh->l, depth + 1);
  }

  if(bvh->r) {
    print_bvh(bvh->r, depth + 1);
  }
}

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
    std::cerr << "[tinyobj error] " << err << std::endl;
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
	this->vertices[idx.vertex_index] = vertex;

	if(attrib.normals.size()) {
	  falg::Vec3 normal(attrib.normals[3 * idx.vertex_index + 0],
			  attrib.normals[3 * idx.vertex_index + 1],
			  attrib.normals[3 * idx.vertex_index + 2]);

	  this->normals[idx.vertex_index] = normal;
	}
	if(attrib.texcoords.size()) {
	  falg::Vec2 uv(attrib.texcoords[2 * idx.vertex_index + 0],
			attrib.texcoords[2 * idx.vertex_index + 1]);
	  this->uvs[idx.vertex_index] = uv;
	}


	indices.push_back(idx.vertex_index);
      }

      index_offset += fv;
    }
  }

  this->num_triangles = indices.size() / 3;

  std::cout << "Num triangles = " << this->num_triangles << std::endl;

  std::vector<int> indices_range(this->num_triangles);
  for(int i = 0; i < this->num_triangles; i++) {
    indices_range[i] = i;
  }



  std::cout << "[TriangleHitable()] Inds: " << indices_range.size() << " elems, first = " << indices_range[0] << ", last: " << (*(indices_range.end() - 1)) << std::endl;

  std::cout << "Constructing BVH tree for " << file_name << std::endl;
  this->bvh_root = this->construct_bvh_tree(indices_range);
  std::cout << "Finished constructing BVH" << std::endl;

  // print_bvh(this->bvh_root);
}

TriangleHitable::~TriangleHitable() {
  TriangleHitable::deconstruct_bvh_tree(this->bvh_root);
}

void TriangleHitable::deconstruct_bvh_tree(TriangleBVH* node) {
  if (node->num_inds) {
    delete[] node->inds;
  }

  if (node->l) {
    TriangleHitable::deconstruct_bvh_tree(node->l);
  }

  if (node->r) {
    TriangleHitable::deconstruct_bvh_tree(node->r);
  }

  delete node;
}

// Compute minimum coord for each triangle
void TriangleHitable::fill_triangle_min_coord(std::vector<std::pair<float, int> >& min_coords,
					      const std::vector<int>& inds,
					      int coord) {
  for(unsigned int i = 0; i < inds.size(); i++) {
    float minx = -1e18;
    minx = std::min(minx, this->vertices[this->indices[3 * inds[i] + 0]][coord]);
    minx = std::min(minx, this->vertices[this->indices[3 * inds[i] + 1]][coord]);
    minx = std::min(minx, this->vertices[this->indices[3 * inds[i] + 2]][coord]);
    min_coords[i] = std::make_pair(minx, inds[i]);
  }
}

TriangleBVH* TriangleHitable::construct_bvh_tree(const std::vector<int>& inds) {


  std::cout << "[construct] Inds: " << inds.size() << " elems, first = " << inds[0] << ", last: " << (*(inds.end() - 1)) << std::endl;

  TriangleBVH* bvh = new TriangleBVH;

  bvh->box = this->compute_aabb(inds);
  std::cout << "Computed bounding box bvh " << bvh->box.min().str() << ", " << bvh->box.max().str() << std::endl;

  if(inds.size() <= 2) {
    bvh->inds = new int[inds.size()];
    bvh->num_inds = inds.size();
    std::cout << "Num inds in computing bvh: " << inds.size() << std::endl;

    memcpy(bvh->inds, inds.data(), bvh->num_inds * sizeof(int));
    return bvh;
  }

  std::vector<std::pair<float, int> > x_scores(inds.size());
  std::vector<std::pair<float, int> > y_scores(inds.size());
  std::vector<std::pair<float, int> > z_scores(inds.size());

  this->fill_triangle_min_coord(x_scores, inds, 0);
  this->fill_triangle_min_coord(y_scores, inds, 1);
  this->fill_triangle_min_coord(z_scores, inds, 2);

  // Sort triangles along the three axes
  std::sort(x_scores.begin(), x_scores.end());
  std::sort(y_scores.begin(), y_scores.end());
  std::sort(z_scores.begin(), z_scores.end());

  Aabb bx0, bx1, by0, by1, bz0, bz1;

  // Compute axis-aligned bounding boxes dividing each sorted array in two
  this->compute_separating_boxes(x_scores, bx0, bx1);
  this->compute_separating_boxes(y_scores, by0, by1);
  this->compute_separating_boxes(z_scores, bz0, bz1);

  // Determine how good the separation is
  float x_score = separation_score(bx0, bx1);
  float y_score = separation_score(by0, by1);
  float z_score = separation_score(bz0, bz1);

  float max_score = std::max(x_score, std::max(y_score, z_score));


  std::vector<std::pair<float, int> >* vecref;

  if(max_score == x_score) {
    vecref = &x_scores;
    std::cout << "Dividing by x-axis" << std::endl;
  } else if (max_score == y_score) {
    vecref = &y_scores;
    std::cout << "Dividing by y-axis" << std::endl;
  } else if (max_score == z_score) {
    vecref = &z_scores;
    std::cout << "Dividing by z-axis" << std::endl;
  } else {
    std::cerr << "WHaT yOu NeveR pLAy TuBEr SiMULaToR? Also, numeric precision works differently, I guess" << std::endl;
    exit(-1);
  }


  unsigned int halfway = inds.size() / 2;

  std::vector<int> vec0(halfway);
  std::vector<int> vec1(inds.size() - halfway);

  for(unsigned int i = 0; i < inds.size(); i++) {
    if(i < halfway) {
      vec0[i] = (*vecref)[i].second;
    } else {
      vec1[i - halfway] = (*vecref)[i].second;
    }
  }

  bvh->l = construct_bvh_tree(vec0);
  bvh->r = construct_bvh_tree(vec1);

  return bvh;
}

void TriangleHitable::compute_separating_boxes(const std::vector<std::pair<float, int> >& vals,
					       Aabb& b0,
					       Aabb& b1) {
  unsigned int half_way = vals.size() / 2;
  std::vector<int> vec0(half_way);
  std::vector<int> vec1(vals.size() - half_way);

  for(unsigned int i = 0; i < vals.size(); i++) {
    if(i < half_way) {
      vec0[i] = vals[i].second;
    } else {
      vec1[i - half_way] = vals[i].second;
    }
  }

  b0 = this->compute_aabb(vec0);
  b1 = this->compute_aabb(vec1);
}

Aabb TriangleHitable::compute_aabb(const std::vector<int>& inds) const {
  Aabb box;
  for(unsigned int i = 0; i < inds.size(); i++) {
    for(int j = 0; j < 3; j++) {
      int ind = this->indices[3 * inds[i] + j];
      box.add(this->vertices[ind]);
    }
  }

  return box;
}

float TriangleHitable::separation_score(const Aabb& b0, const Aabb& b1) {
  falg::Vec3 diff0 = b0.min() - b1.max();
  float max0 = std::max(diff0[0],
			std::max(diff0[1], diff0[2]));
  falg::Vec3 diff1 = b1.min() - b0.max();
  float max1 = std::max(diff1[0],
			std::max(diff1[1], diff1[2]));

  return std::max(max0, max1);
}

bool TriangleHitable::hit_triangle(const Ray& r, float tmin, float tmax, hit_record& rec, int ind) const {
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

  float det = falg::dot(p, e1);
  if(std::abs(det) <= 1e-7) {
    // Ray and triangle nearly parallel
    return false;
  }

  Vec3 tuv = (1.0 / det) * Vec3(falg::dot(q, e2), falg::dot(p, tvec), falg::dot(q, dvec));

  float t = tuv[0];
  float u = tuv[1];
  float v = tuv[2];

  if(t <= tmin || t >= tmax || (u + v) > 1 || v < 0 || u < 0) {
    return false;
  }

  rec.t = t;
  rec.p = r.origin() + dvec * t;
  rec.normal = (this->normals[this->indices[3 * ind + 0]] * (1 - u - v) +
    this->normals[this->indices[3 * ind + 1]] * u +
		this->normals[this->indices[3 * ind + 2]] * v).normalized();

  rec.mat_ptr = this->mat_ptr;
  rec.u = u;
  rec.v = v;
  return true;
}

bool TriangleHitable::hit_triangle_bvh(TriangleBVH* node, const Ray& r,
				       float tmin, float tmax, hit_record& rec,
				       int depth) const {

  if (node->box.hit(r, tmin, tmax)) {

    bool bb = false;

    if(node->num_inds) {
      for(int i = 0; i < node->num_inds; i++) {
	if(this->hit_triangle(r, tmin, tmax, rec, node->inds[i])) {
	  tmax = rec.t;
	  bb = true;
	}
      }
    }

    if(node->l || node->r) {
      hit_record rl, rr;

      // Ensure rl.t is set
      rl.t = tmax;

      bool hl = node->l ? hit_triangle_bvh(node->l, r, tmin, tmax, rl, depth + 1) : false;

      // Update tmax, so we may skip hr if we're lucky
      tmax = rl.t;

      bool hr = node->r ? hit_triangle_bvh(node->r, r, tmin, tmax, rr, depth + 1) : false;

      if(hl && hr) {
	if(rl.t < rr.t) {
	  rec = rl;
	} else {
	  rec = rr;
	}
	return true;
      } else if (hl) {
	rec = rl;
	return true;
      } else if (hr) {
	rec = rr;
	return true;
      } else {
	return bb;
      }

    } else {
      return bb;
    }
  }

  return false;
}

bool TriangleHitable::hit(const Ray& r, float tmin, float tmax, hit_record& rec) const {
  // Naive intersection implementation
  /* bool hit_anything = false;
  for(int i = 0; i < this->num_triangles; i++) {
    if(hit_triangle(r, tmin, tmax, rec, i)) {
      tmax = rec.t;
      hit_anything = true;
    }
  }
  return hit_anything; */

  // Optimized bvh
  return this->hit_triangle_bvh(bvh_root, r, tmin, tmax, rec, 0);
}

bool TriangleHitable::bounding_box(float t0, float t1, Aabb& box) const {

  std::vector<int> inds(this->num_triangles);
  for(int i = 0; i < this->num_triangles; i++) {
    inds[i] = i;
  }

  box = this->compute_aabb(inds);

  return true;
}
