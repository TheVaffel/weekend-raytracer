#include "ray.hpp"
#include "perlin.hpp"

class Texture {
public:
  virtual vec3 value(float u, float v, const vec3& p) const = 0;
};

class ConstantTexture : public Texture {
public:
  ConstantTexture() {}
  ConstantTexture(const vec3& c) : color(c) {}

  virtual vec3 value(float u, float v, const vec3& p) const {
    return color;
  }

  vec3 color;
};

class CheckerTexture : public Texture {
public:
  CheckerTexture() {}
  CheckerTexture(Texture* t0,
		 Texture* t1) : even(t0), odd(t1) { }

  virtual vec3 value(float u, float v, const vec3& p) const {
    float sines = sin(10 * p.x()) * sin(10 * p.y()) * sin(10 * p.z());
    if(sines < 0) {
      return odd->value(u, v, p);
    } else {
      return even->value(u, v, p);
    }
  }
  
  Texture *even, *odd;
};

class NoiseTexture : public Texture {
public:
  NoiseTexture(unidist& dist) : noise(dist), scale(1.0f) { }
  NoiseTexture(unidist& dist, float scale) : noise(dist), scale(scale) {}
  virtual vec3 value(float u, float v, const vec3& p) const {
    return vec3(1, 1, 1) * 0.5 * (1 + sin(scale * p.z() + 10 * noise.turb(p))); // noise.noise(p * scale);
  }

  Perlin noise;
  float scale;
};
