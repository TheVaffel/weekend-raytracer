#ifndef _PERLIN_HPP
#define _PERLIN_HPP

#include "ray.hpp"
#include "utils.hpp"

float perlin_interp(vec3 c[2][2][2], float u, float v, float w) {
  float accum = 0;

  float uu = u * u * (3 - 2 * u);
  float vv = v * v * (3 - 2 * v);
  float ww = w * w * (3 - 2 * w);

  for(int i = 0; i < 2; i++) {
    for(int j = 0; j < 2; j++) {
      for(int k = 0; k < 2; k++) {
	vec3 weight_v (u - i, v - j, w - k);

	accum +=
            (i * uu + (1 - i) * (1 - uu)) *
            (j * vv + (1 - j) * (1 - vv)) *
            (k * ww + (1 - k) * (1 - ww)) * falg::dot(c[i][j][k], weight_v);
      }
    }
  }

  return (accum + 1.0) / 2.0;
}

vec3* perlin_generate(unidist& dist) {
  const int num = 256;
  vec3 * p = new vec3[num];
  for(int i = 0; i < num; ++i) {
    p[i] = vec3(-1 + 2 * dist.get(), -1 + 2 * dist.get(), -1 + 2 * dist.get()).normalized();
  }
  return p;
}

void permute(int *p, int n, unidist& dist) {
  for(int i = n - 1; i >= 0; i--) {
    int target = int(dist.get() * (i + 1));
    int tmp = p[i];
    p[i] = p[target];
    p[target] = tmp;
  }

  return;
}

static int* perlin_generate_perm(unidist& dist) {
  const int num = 256;
  int * p = new int[num];
  for (int i = 0; i < num; i++) {
    p[i] = i;
  }
  permute(p, num, dist);
  return p;
}

class Perlin {
public:

  Perlin(unidist& dist) {
    ranvec = perlin_generate(dist);

    perm_x = perlin_generate_perm(dist);
    perm_y = perlin_generate_perm(dist);
    perm_z = perlin_generate_perm(dist);
  }


  float turb(const vec3& p, int depth=7) const {
    float accum = 0;
    vec3 temp_p = p;
    float weight = 1.0;
    for(int i = 0; i < depth; i++) {
      accum += weight * noise(temp_p);
      weight *= 0.5;
      temp_p *= 2;
    }
    return std::abs(accum);
  }

  float noise(const vec3& p) const {
    float u = p.x() - floor(p.x());
    float v = p.y() - floor(p.y());
    float w = p.z() - floor(p.z());

    int i = floor(p.x());
    int j = floor(p.y());
    int k = floor(p.z());

    vec3 c[2][2][2];

    for(int di = 0; di < 2; di++) {
      for(int dj = 0; dj < 2; dj++) {
	for(int dk = 0; dk < 2; dk++) {
	  c[di][dj][dk] = ranvec[perm_x[(i + di) & 255] ^ perm_y[(j + dj) & 255] ^ perm_z[(k + dk) & 255]];
	}
      }
    }

    // return ranfloat[perm_x[i] ^ perm_y[j] ^ perm_z[k]];
    return perlin_interp(c, u, v, w);
  }

  vec3 *ranvec;
  int *perm_x;
  int *perm_y;
  int *perm_z;
};



#endif // _PERLIN_HPP
