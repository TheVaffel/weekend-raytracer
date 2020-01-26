#ifndef INCLUDE_RAY_HPP
#define INCLUDE_RAY_HPP

#include <FlatAlg.hpp>

typedef Vector3 vec3;

class Ray {
public:
  Ray() {}
  Ray(const Vector3& orig, const Vector3& dir, float time) : orig(orig), dir(dir), _time(time) {}
  Vector3 origin() const { return orig; }
  Vector3 direction() const { return dir; }
  float time() const { return _time; }
  Vector3 point_at_parameter(float t) const {return orig + t * dir; }

  Vector3 orig;
  Vector3 dir;
  float _time;
};

#endif // ndef INCLUDE_RAY_HPP
