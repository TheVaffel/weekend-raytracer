#ifndef INCLUDE_RAY_HPP
#define INCLUDE_RAY_HPP

#include <FlatAlg.hpp>

typedef Vector3 vec3;

class Ray {
public:
  Ray() {}
  Ray(const Vector3& orig, const Vector3& dir) : orig(orig), dir(dir) {}
  Vector3 origin() const { return orig; }
  Vector3 direction() const { return dir; }
  Vector3 point_at_parameter(float t) const {return orig + t * dir; }

  Vector3 orig;
  Vector3 dir;
};

#endif // ndef INCLUDE_RAY_HPP
