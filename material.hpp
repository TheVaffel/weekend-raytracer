#ifndef INCLUDE_MATERIAL_HPP
#define INCLUDE_MATERIAL_HPP

#include <algorithm>

#include "ray.hpp"
#include "texture.hpp"
#include "utils.hpp"

class Material {
public:
  virtual bool scatter(const Ray& r_in, const hit_record& rec, vec3& attenuation, Ray& scattered, unidist& dist) const = 0;
  virtual vec3 emitted(float u, float v, const vec3& p) const {
    return vec3(0, 0, 0);
  }
};


class Lambertian : public Material {
public:
  Lambertian(Texture* a) : albedo(a) {}
  virtual bool scatter(const Ray& r_in, const hit_record& rec, vec3& attenuation, Ray& scattered, unidist& dist) const {
    vec3 target = rec.p + rec.normal + random_in_unit_sphere(dist);
    scattered = Ray(rec.p, target - rec.p, r_in.time());
    attenuation = albedo->value(rec.u, rec.v, rec.p);
    return true;
  }

  Texture* albedo;
};

vec3 reflect(const vec3& v, const vec3& n) {
  return v - 2 * (v * n) * n;
}

class Metal : public Material {
public:
  Metal(const vec3& a, float fuzziness) : albedo(a) { fuzz = std::min(fuzziness, 1.0f);}
  virtual bool scatter(const Ray& r_in, const hit_record& rec, vec3& attenuation, Ray& scattered, unidist& dist) const {
    vec3 reflected = reflect(r_in.direction().normalized(), rec.normal);
    scattered = Ray(rec.p, reflected + fuzz * random_in_unit_sphere(dist), r_in.time());
    attenuation = albedo;
    return scattered.direction() * rec.normal > 0;
  }
  vec3 albedo;
  float fuzz;
};

bool refract(const vec3& v, const vec3& n, float ni_over_nt, vec3& refracted) {
  vec3 uv = v.normalized();

  float dt = uv * n;
  float discriminant = 1.0 - ni_over_nt * ni_over_nt * (1 - dt * dt);
  if (discriminant > 0) {
    refracted = ni_over_nt * (uv - n * dt) - n * sqrt(discriminant);
    return true;
  } else {
    return false;
  }
}

float schlick(float cosine, float ref_idx) {
  float r0 = (1 - ref_idx) / (1 + ref_idx);
  r0 = r0 * r0;
  return r0 + (1 - r0) * pow((1 - cosine), 5);
}


class Dielectric : public Material {
public:
  Dielectric(float ri) : ref_idx(ri) {}
  virtual bool scatter(const Ray& r_in, const hit_record& rec, vec3& attenuation, Ray& scattered, unidist& dist) const {
    vec3 outward_normal;
    vec3 reflected = reflect(r_in.direction(), rec.normal);
    float ni_over_nt;
    attenuation = vec3(1.0, 1.0, 1.0);
    vec3 refracted;
    float reflect_prob;
    float cosine;
    
    if (r_in.direction() * rec.normal > 0) {
      outward_normal = -rec.normal;
      ni_over_nt = ref_idx;
      cosine = ref_idx * (r_in.direction() * rec.normal) / r_in.direction().norm();
    } else {
      outward_normal = rec.normal;
      ni_over_nt = 1.0 / ref_idx;
      cosine = -r_in.direction() * rec.normal / r_in.direction().norm();
    }

    if (refract(r_in.direction(), outward_normal, ni_over_nt, refracted)) {
      reflect_prob = schlick(cosine, ref_idx);
    } else {
      reflect_prob = 1.0;
    }

    if (dist.get() < reflect_prob) {
      scattered = Ray(rec.p, reflected, r_in.time());
    } else {
      scattered = Ray(rec.p, refracted, r_in.time());
    }

    return true;
  }

  float ref_idx;
};

class DiffuseLight : public Material {
public:
  DiffuseLight(Texture* tex) : emit(tex) { }
  virtual bool scatter(const Ray& r_in, const hit_record& rec, vec3& attenuation, Ray& scattered, unidist& dist) const {
    return false;
  }
  virtual vec3 emitted(float u, float v, const vec3& p) const {
    return emit->value(u, v, p);
  }

  Texture* emit;
};

class Isotropic : public Material {
public:
  Isotropic(Texture* tex) : albedo(tex) {}
  virtual bool scatter(const Ray& r, const hit_record& rec,
		       vec3& attenuation, Ray& scattered, unidist& dist) const {
    scattered = Ray(rec.p, random_in_unit_sphere(dist), r.time());
    attenuation = albedo->value(rec.u, rec.v, rec.p);
    return true;
  }

  Texture *albedo;
};

#endif // INCLUDE_MATERIAL_HPP
