#ifndef _TEXTURE_HPP
#define _TEXTURE_HPP

#include "ray.hpp"
#include "perlin.hpp"

#include "OpenImageIO/imageio.h"

namespace OpenImageIO = OIIO;

void get_sphere_uv(const vec3& p, float *u, float *v) {
  float phi = atan2(p.z(), p.x());
  float theta = asin(p.y());
  *u = 1 - (phi + F_PI) / (2 * F_PI);
  *v = (theta + F_PI / 2) / F_PI;
}

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

class  ImageTexture : public Texture {
public:
  ImageTexture() {}
  ImageTexture(float *pixels, int a, int b) : data(pixels), nx(a), ny(b), nchannels(3) {}
  ImageTexture(const std::string& image_file) {
    OpenImageIO::ImageInput *imin = OpenImageIO::ImageInput::open(image_file);
    if(!imin) {
      std::cerr << "Could not open image file " << image_file << ", exiting" << std::endl;
      exit(-1);
    }
    const OpenImageIO::ImageSpec& spec = imin->spec();
    int width = spec.width;
    int height = spec.height;
    int channels = spec.nchannels;
    this->data = new float[width * height * channels];
    this->nx = width;
    this->ny = height;
    this->nchannels = channels;
    imin->read_image(OpenImageIO::TypeDesc::FLOAT, data);
    imin->close();
  }
  virtual vec3 value(float u, float v, const vec3& p) const;
  float* data;
  int nx, ny;

  int nchannels;
};

vec3 ImageTexture::value(float u, float v, const vec3& p) const {
  int i = u * nx;
  int j = (1 - v) * ny - 0.001;
  if (i < 0) i = 0;
  if (j < 0) j = 0;
  if (i > nx - 1) i = nx - 1;
  if (j > ny - 1) j = ny - 1;

  float r = data[this->nchannels * (i + nx * j) + 0];
  float g = data[this->nchannels * (i + nx * j) + 1];
  float b = data[this->nchannels * (i + nx * j) + 2];

  return vec3(r, g, b);
}

#endif // _TEXTURE_HPP
