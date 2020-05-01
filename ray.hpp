#ifndef INCLUDE_RAY_HPP
#define INCLUDE_RAY_HPP

#include <FlatAlg.hpp>

typedef falg::Vec3 vec3;

class Ray {
public:
  Ray() {}
  Ray(const vec3& orig, const vec3& dir, float time) : orig(orig), dir(dir), _time(time) {}
  vec3 origin() const { return orig; }
  vec3 direction() const { return dir; }
  float time() const { return _time; }
  vec3 point_at_parameter(float t) const {return orig + t * dir; }

  vec3 orig;
  vec3 dir;
  float _time;
};

#endif // ndef INCLUDE_RAY_HPP
